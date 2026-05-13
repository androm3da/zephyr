# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: Apache-2.0

"""
Shared core logic for Zephyr RTOS debug plugins.

This module contains all the kernel-object walking, offset resolution,
and formatting logic used by both the LLDB and GDB frontends.  It is
debugger-agnostic: every target access goes through a ``Backend``
instance supplied by the caller.

Frontends must subclass ``Backend`` and implement its abstract methods,
then pass the backend to the ``cmd_*`` functions which return lists of
output lines.
"""


# ---------------------------------------------------------------------------
# Backend interface (implemented by LLDB / GDB frontends)
# ---------------------------------------------------------------------------

class Backend:
    """Abstract interface to the debugger's target inspection APIs."""

    def read_u32(self, addr):
        """Read a uint32_t from the target.  Return int or None."""
        raise NotImplementedError

    def read_u8(self, addr):
        """Read a uint8_t from the target.  Return int or None."""
        raise NotImplementedError

    def read_ptr(self, addr):
        """Read a pointer-sized value from the target.  Return int or None."""
        raise NotImplementedError

    def read_string(self, addr, max_len=64):
        """Read a NUL-terminated C string.  Return str or None."""
        raise NotImplementedError

    def get_symbol_addr(self, name):
        """Return the load address of a global symbol, or None."""
        raise NotImplementedError

    def get_member_offset(self, type_name, member):
        """Return the byte offset of *member* inside *type_name*, or None.

        Must recurse into anonymous unions/structs.
        """
        raise NotImplementedError

    def get_type_size(self, type_name):
        """Return the size in bytes of *type_name*, or None."""
        raise NotImplementedError

    def eval_expr(self, expr):
        """Evaluate *expr* and return its value as an integer address, or None."""
        raise NotImplementedError

    def resolve_symbol(self, addr):
        """Return the symbol name for *addr*, or None."""
        raise NotImplementedError

    def is_big_endian(self):
        """Return True if the target is big-endian."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Thread state bitmask definitions (from include/zephyr/kernel_structs.h)
# ---------------------------------------------------------------------------

_THREAD_DUMMY = 1 << 0
_THREAD_PENDING = 1 << 1
_THREAD_SLEEPING = 1 << 2
_THREAD_DEAD = 1 << 3
_THREAD_SUSPENDED = 1 << 4
_THREAD_ABORTING = 1 << 5
_THREAD_SUSPENDING = 1 << 6
_THREAD_QUEUED = 1 << 7

_STATE_NAMES = [
    (_THREAD_DUMMY, "DUMMY"),
    (_THREAD_PENDING, "PENDING"),
    (_THREAD_SLEEPING, "SLEEPING"),
    (_THREAD_DEAD, "DEAD"),
    (_THREAD_SUSPENDED, "SUSPENDED"),
    (_THREAD_ABORTING, "ABORTING"),
    (_THREAD_SUSPENDING, "SUSPENDING"),
    (_THREAD_QUEUED, "QUEUED"),
]


def state_str(state_val):
    """Convert a thread_state bitmask to a human-readable string."""
    if state_val == 0:
        return "READY"
    parts = [name for bit, name in _STATE_NAMES if state_val & bit]
    return "|".join(parts) if parts else f"0x{state_val:02x}"


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _read_i8(backend, addr):
    """Read a signed int8 via the backend."""
    val = backend.read_u8(addr)
    if val is None:
        return None
    return val if val < 128 else val - 256


# ---------------------------------------------------------------------------
# Cached offset table (populated lazily from DWARF)
# ---------------------------------------------------------------------------

class OffsetCache:
    """Cache struct member offsets looked up from debug info."""

    def __init__(self):
        self._cache = {}
        self._size_cache = {}

    def offset(self, backend, type_name, member):
        key = (type_name, member)
        if key not in self._cache:
            self._cache[key] = backend.get_member_offset(type_name, member)
        return self._cache[key]

    def size(self, backend, type_name):
        if type_name not in self._size_cache:
            self._size_cache[type_name] = backend.get_type_size(type_name)
        return self._size_cache[type_name]

    def clear(self):
        self._cache.clear()
        self._size_cache.clear()


# Module-level singleton shared by all commands
_offsets = OffsetCache()


# ---------------------------------------------------------------------------
# Thread walking
# ---------------------------------------------------------------------------

def _get_kernel_addr(backend):
    """Return the address of the global _kernel struct."""
    return backend.get_symbol_addr("_kernel")


def get_current_thread(backend):
    """Return the address of the currently executing thread."""
    kernel = _get_kernel_addr(backend)
    if kernel is None:
        return None
    off_cpus = _offsets.offset(backend, "z_kernel", "cpus")
    off_current = _offsets.offset(backend, "_cpu", "current")
    if off_cpus is None or off_current is None:
        return None
    return backend.read_ptr(kernel + off_cpus + off_current)


def get_idle_thread(backend):
    """Return the address of the idle thread (cpus[0].idle_thread)."""
    kernel = _get_kernel_addr(backend)
    if kernel is None:
        return None
    off_cpus = _offsets.offset(backend, "z_kernel", "cpus")
    off_idle = _offsets.offset(backend, "_cpu", "idle_thread")
    if off_cpus is None or off_idle is None:
        return None
    return backend.read_ptr(kernel + off_cpus + off_idle)


def walk_thread_list(backend):
    """Walk the kernel thread monitor list (_kernel.threads).

    Requires CONFIG_THREAD_MONITOR=y.  Returns a list of thread addresses.
    Falls back to gathering threads from current, idle, and ready queue.
    """
    kernel = _get_kernel_addr(backend)
    if kernel is None:
        return []

    off_threads = _offsets.offset(backend, "z_kernel", "threads")
    off_next = _offsets.offset(backend, "k_thread", "next_thread")

    if off_threads is not None and off_next is not None:
        threads = []
        ptr = backend.read_ptr(kernel + off_threads)
        seen = set()
        while ptr and ptr != 0 and ptr not in seen:
            seen.add(ptr)
            threads.append(ptr)
            ptr = backend.read_ptr(ptr + off_next)
        if threads:
            return threads

    # Fallback: gather what we can without CONFIG_THREAD_MONITOR
    seen = set()
    threads = []

    current = get_current_thread(backend)
    if current and current != 0:
        seen.add(current)
        threads.append(current)

    idle = get_idle_thread(backend)
    if idle and idle != 0 and idle not in seen:
        seen.add(idle)
        threads.append(idle)

    for t in get_readyq_threads(backend):
        if t not in seen:
            seen.add(t)
            threads.append(t)

    return threads


def thread_name(backend, thread_addr):
    """Read the name of a thread (CONFIG_THREAD_NAME)."""
    off_name = _offsets.offset(backend, "k_thread", "name")
    if off_name is None:
        return None
    name = backend.read_string(thread_addr + off_name)
    if name and len(name) > 0:
        return name
    return None


def thread_state(backend, thread_addr):
    """Read the thread_state field from _thread_base."""
    off_base = _offsets.offset(backend, "k_thread", "base")
    off_state = _offsets.offset(backend, "_thread_base", "thread_state")
    if off_base is None or off_state is None:
        return None
    return backend.read_u8(thread_addr + off_base + off_state)


def thread_prio(backend, thread_addr):
    """Read the priority from _thread_base.

    The prio field lives inside an anonymous struct within the preempt
    union, which DWARF may not expose as a direct child.  Fall back to
    reading the preempt uint16 and extracting prio from the low byte
    (little-endian) or high byte (big-endian).
    """
    off_base = _offsets.offset(backend, "k_thread", "base")
    if off_base is None:
        return None

    # Try direct DWARF lookup first
    off_prio = _offsets.offset(backend, "_thread_base", "prio")
    if off_prio is not None:
        return _read_i8(backend, thread_addr + off_base + off_prio)

    # Fallback: read the preempt union as uint16 and extract prio byte
    off_preempt = _offsets.offset(backend, "_thread_base", "preempt")
    if off_preempt is not None:
        val = backend.read_u32(thread_addr + off_base + off_preempt)
        if val is None:
            return None
        preempt = val & 0xFFFF
        if backend.is_big_endian():
            prio_u8 = (preempt >> 8) & 0xFF
        else:
            prio_u8 = preempt & 0xFF
        return prio_u8 if prio_u8 < 128 else prio_u8 - 256

    return None


def thread_stack_info(backend, thread_addr):
    """Read stack_info (start, size) from k_thread."""
    off_si = _offsets.offset(backend, "k_thread", "stack_info")
    if off_si is None:
        return None, None
    off_start = _offsets.offset(backend, "_thread_stack_info", "start")
    off_size = _offsets.offset(backend, "_thread_stack_info", "size")
    if off_start is None or off_size is None:
        return None, None
    start = backend.read_ptr(thread_addr + off_si + off_start)
    size = backend.read_ptr(thread_addr + off_si + off_size)
    return start, size


def thread_entry(backend, thread_addr):
    """Read the thread entry point (CONFIG_THREAD_MONITOR)."""
    off_entry = _offsets.offset(backend, "k_thread", "entry")
    if off_entry is None:
        return None
    off_pentry = _offsets.offset(backend, "__thread_entry", "pEntry")
    if off_pentry is None:
        return None
    return backend.read_ptr(thread_addr + off_entry + off_pentry)


# ---------------------------------------------------------------------------
# Wait queue / dlist walking
# ---------------------------------------------------------------------------

def walk_waitq(backend, waitq_addr):
    """Walk a _wait_q_t and return addresses of waiting threads."""
    off_waitq = _offsets.offset(backend, "_wait_q_t", "waitq")
    if off_waitq is None:
        return []
    return _walk_dlist_threads(backend, waitq_addr + off_waitq)


def _walk_dlist_threads(backend, list_addr):
    """Walk a sys_dlist_t where nodes are k_thread.base.qnode_dlist.

    Returns list of k_thread addresses.
    """
    off_base = _offsets.offset(backend, "k_thread", "base")
    off_qnode = _offsets.offset(backend, "_thread_base", "qnode_dlist")
    if off_base is None or off_qnode is None:
        return []

    node_to_thread_off = off_base + off_qnode
    threads = []

    head = backend.read_ptr(list_addr)
    if head is None or head == 0 or head == list_addr:
        return []

    node = head
    seen = set()
    while node and node != list_addr and node not in seen:
        seen.add(node)
        threads.append(node - node_to_thread_off)
        node = backend.read_ptr(node)

    return threads


# ---------------------------------------------------------------------------
# Ready queue inspection
# ---------------------------------------------------------------------------

def get_readyq_cache(backend):
    """Get the cached next-to-run thread from the ready queue."""
    kernel = _get_kernel_addr(backend)
    if kernel is None:
        return None
    off_rq = _offsets.offset(backend, "z_kernel", "ready_q")
    if off_rq is None:
        return None
    off_cache = _offsets.offset(backend, "_ready_q", "cache")
    if off_cache is None:
        return None
    return backend.read_ptr(kernel + off_rq + off_cache)


def get_readyq_threads(backend):
    """Walk the ready queue run queue (CONFIG_SCHED_SIMPLE dlist)."""
    kernel = _get_kernel_addr(backend)
    if kernel is None:
        return []
    off_rq = _offsets.offset(backend, "z_kernel", "ready_q")
    if off_rq is None:
        return []
    off_runq = _offsets.offset(backend, "_ready_q", "runq")
    if off_runq is None:
        return []
    return _walk_dlist_threads(backend, kernel + off_rq + off_runq)


# ---------------------------------------------------------------------------
# Output formatting helpers
# ---------------------------------------------------------------------------

def format_thread_line(backend, thread_addr, current_thread):
    """Format a single-line summary of a thread."""
    marker = ">" if thread_addr == current_thread else " "
    name = thread_name(backend, thread_addr) or f"0x{thread_addr:08x}"
    state = thread_state(backend, thread_addr)
    prio = thread_prio(backend, thread_addr)
    state_s = state_str(state) if state is not None else "?"
    prio_s = str(prio) if prio is not None else "?"

    stack_start, stack_size = thread_stack_info(backend, thread_addr)
    stack_s = ""
    if stack_start is not None and stack_size is not None:
        stack_s = f"  stack: 0x{stack_start:08x} size={stack_size}"

    entry = thread_entry(backend, thread_addr)
    entry_s = ""
    if entry is not None and entry != 0:
        sym = backend.resolve_symbol(entry)
        if sym:
            entry_s = f"  entry: {sym}"
        else:
            entry_s = f"  entry: 0x{entry:08x}"

    return (f"{marker} 0x{thread_addr:08x}  {name:<20s}  "
            f"state={state_s:<16s}  prio={prio_s:>3s}"
            f"{stack_s}{entry_s}")


def format_thread_detail(backend, thread_addr, current_thread):
    """Format detailed info about a single thread."""
    lines = []
    marker = " (CURRENT)" if thread_addr == current_thread else ""
    name = thread_name(backend, thread_addr) or "(unnamed)"
    lines.append(f"Thread 0x{thread_addr:08x} \"{name}\"{marker}")
    lines.append(f"  {'='*60}")

    state = thread_state(backend, thread_addr)
    prio = thread_prio(backend, thread_addr)
    lines.append(f"  State:    {state_str(state) if state is not None else '?'}")
    lines.append(f"  Priority: {prio if prio is not None else '?'}")

    entry = thread_entry(backend, thread_addr)
    if entry is not None and entry != 0:
        sym = backend.resolve_symbol(entry)
        sym_name = sym if sym else f"0x{entry:08x}"
        lines.append(f"  Entry:    {sym_name}")

    stack_start, stack_size = thread_stack_info(backend, thread_addr)
    if stack_start is not None and stack_size is not None:
        lines.append(f"  Stack:    0x{stack_start:08x} - "
                     f"0x{stack_start + stack_size:08x} "
                     f"(size={stack_size})")

    return lines


def format_waitq(backend, waitq_addr, label="Wait queue"):
    """Format the contents of a wait queue."""
    threads = walk_waitq(backend, waitq_addr)
    lines = [f"{label}: {len(threads)} waiting thread(s)"]
    for t in threads:
        name = thread_name(backend, t) or f"0x{t:08x}"
        prio = thread_prio(backend, t)
        prio_s = str(prio) if prio is not None else "?"
        lines.append(f"  0x{t:08x}  {name:<20s}  prio={prio_s}")
    return lines


# ---------------------------------------------------------------------------
# Command implementations (return lists of output lines)
# ---------------------------------------------------------------------------

# Subcommand table: name -> (function, description)
SUBCMDS = {
    "threads": "List all Zephyr threads",
    "current": "Show current thread details",
    "readyq": "Show the ready queue",
    "mutex": "Inspect a k_mutex (usage: zephyr mutex <expr>)",
    "sem": "Inspect a k_sem (usage: zephyr sem <expr>)",
    "waitq": "Show wait queue (usage: zephyr waitq <expr>)",
}


def cmd_help():
    """Return help text lines."""
    lines = [
        "Zephyr RTOS Debug Commands",
        "=" * 45,
    ]
    for name, desc in SUBCMDS.items():
        lines.append(f"  zephyr {name:<10s} - {desc}")
    lines.append("")
    lines.append("Example:")
    lines.append("  zephyr threads")
    lines.append("  zephyr mutex my_mutex")
    return lines


def cmd_threads(backend):
    """List all Zephyr threads."""
    current = get_current_thread(backend)
    threads = walk_thread_list(backend)

    if not threads:
        return [
            "No threads found. Enable CONFIG_THREAD_MONITOR=y for full "
            "thread list, or ensure target is running/stopped with a "
            "valid _kernel."
        ]

    lines = [
        f"{'':1s} {'Address':10s}  {'Name':20s}  {'State':16s}  "
        f"{'Prio':>4s}  Info",
        "-" * 100,
    ]
    for t in threads:
        lines.append(format_thread_line(backend, t, current))
    footer = (f"\nTotal: {len(threads)} thread(s), "
              f"current=0x{current:08x}" if current else "")
    if footer:
        lines.append(footer)
    return lines


def cmd_current(backend):
    """Show detailed info for the current thread."""
    current = get_current_thread(backend)
    if current is None or current == 0:
        return ["Cannot determine current thread."]
    return format_thread_detail(backend, current, current)


def cmd_readyq(backend):
    """Show the Zephyr ready queue."""
    lines = []
    cached = get_readyq_cache(backend)
    current = get_current_thread(backend)

    if cached is not None:
        name = thread_name(backend, cached) or f"0x{cached:08x}"
        lines.append(f"Ready queue cache (next to run): "
                     f"0x{cached:08x} ({name})")
    else:
        lines.append("Ready queue cache: unavailable")

    threads = get_readyq_threads(backend)
    if threads:
        lines.append(f"\nRun queue ({len(threads)} threads):")
        for t in threads:
            line = format_thread_line(backend, t, current)
            lines.append(f"  {line}")
    else:
        lines.append("\nRun queue: empty (or non-dlist scheduler)")

    return lines


def cmd_mutex(backend, expr):
    """Inspect a k_mutex."""
    if not expr:
        return [
            "Usage: zephyr mutex <expression>",
            "  e.g.: zephyr mutex my_mutex",
            "  e.g.: zephyr mutex *(struct k_mutex *)0x12345",
        ]

    mutex_addr = backend.eval_expr(expr)
    if mutex_addr is None:
        return [f"Cannot evaluate '{expr}'"]

    lines = [f"k_mutex @ 0x{mutex_addr:08x}"]

    off_owner = _offsets.offset(backend, "k_mutex", "owner")
    off_lock_count = _offsets.offset(backend, "k_mutex", "lock_count")
    off_orig_prio = _offsets.offset(backend, "k_mutex", "owner_orig_prio")
    off_waitq = _offsets.offset(backend, "k_mutex", "wait_q")

    if off_owner is not None:
        owner = backend.read_ptr(mutex_addr + off_owner)
        if owner and owner != 0:
            name = thread_name(backend, owner) or f"0x{owner:08x}"
            lines.append(f"  Owner:      0x{owner:08x} ({name})")
        else:
            lines.append("  Owner:      (none - unlocked)")

    if off_lock_count is not None:
        lock_count = backend.read_u32(mutex_addr + off_lock_count)
        lines.append(f"  Lock count: {lock_count}")

    if off_orig_prio is not None:
        orig_prio = backend.read_u32(mutex_addr + off_orig_prio)
        if orig_prio is not None and orig_prio > 0x7fffffff:
            orig_prio -= 0x100000000
        lines.append(f"  Owner orig prio: {orig_prio}")

    if off_waitq is not None:
        lines.extend(format_waitq(backend, mutex_addr + off_waitq,
                                  "  Waiters"))

    return lines


def cmd_sem(backend, expr):
    """Inspect a k_sem."""
    if not expr:
        return ["Usage: zephyr sem <expression>"]

    sem_addr = backend.eval_expr(expr)
    if sem_addr is None:
        return [f"Cannot evaluate '{expr}'"]

    lines = [f"k_sem @ 0x{sem_addr:08x}"]

    off_count = _offsets.offset(backend, "k_sem", "count")
    off_limit = _offsets.offset(backend, "k_sem", "limit")
    off_waitq = _offsets.offset(backend, "k_sem", "wait_q")

    if off_count is not None:
        count = backend.read_u32(sem_addr + off_count)
        lines.append(f"  Count: {count}")

    if off_limit is not None:
        limit = backend.read_u32(sem_addr + off_limit)
        lines.append(f"  Limit: {limit}")

    if off_waitq is not None:
        lines.extend(format_waitq(backend, sem_addr + off_waitq,
                                  "  Waiters"))

    return lines


def cmd_waitq(backend, expr):
    """Show threads waiting on a wait queue."""
    if not expr:
        return ["Usage: zephyr waitq <expression>"]

    wq_addr = backend.eval_expr(expr)
    if wq_addr is None:
        return [f"Cannot evaluate '{expr}'"]

    return format_waitq(backend, wq_addr, "Wait queue")
