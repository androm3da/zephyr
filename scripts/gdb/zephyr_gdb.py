# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: Apache-2.0

"""
GDB Python plugin for debugging Zephyr RTOS.

Architecture-independent: all struct layouts are resolved from DWARF debug
info at runtime, so this plugin works on any Zephyr-supported architecture
(Hexagon, ARM, RISC-V, Xtensa, x86, ARC, ...).

Provides commands for inspecting Zephyr kernel objects:
  - zephyr threads    : List all threads with state, priority, and stack info
  - zephyr current    : Show details of the current thread
  - zephyr readyq     : Show the ready queue
  - zephyr mutex EXPR : Inspect a k_mutex
  - zephyr sem EXPR   : Inspect a k_sem
  - zephyr waitq EXPR : Show threads waiting on a wait queue

Usage:
  (gdb) source scripts/gdb/zephyr_gdb.py
  (gdb) zephyr threads
"""

import os
import struct
import sys

import gdb

# Add the shared debug module to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "debug"))
import zephyr_core


# ---------------------------------------------------------------------------
# GDB Backend
# ---------------------------------------------------------------------------

class GdbBackend(zephyr_core.Backend):
    """Backend implementation using the GDB Python API."""

    def __init__(self):
        self._ptr_size = None
        self._big_endian = None

    def _get_ptr_size(self):
        if self._ptr_size is None:
            try:
                void_ptr = gdb.lookup_type("void").pointer()
                self._ptr_size = void_ptr.sizeof
            except gdb.error:
                self._ptr_size = 4
        return self._ptr_size

    def _get_endian(self):
        if self._big_endian is None:
            endian_str = gdb.execute("show endian", to_string=True)
            self._big_endian = "big endian" in endian_str
        return self._big_endian

    def _read_mem(self, addr, size):
        """Read *size* bytes from *addr*, return bytes or None."""
        try:
            inf = gdb.selected_inferior()
            mem = inf.read_memory(addr, size)
            return bytes(mem)
        except gdb.MemoryError:
            return None

    def read_u32(self, addr):
        data = self._read_mem(addr, 4)
        if data is None:
            return None
        fmt = ">I" if self._get_endian() else "<I"
        return struct.unpack(fmt, data)[0]

    def read_u8(self, addr):
        data = self._read_mem(addr, 1)
        if data is None:
            return None
        return data[0]

    def read_ptr(self, addr):
        sz = self._get_ptr_size()
        data = self._read_mem(addr, sz)
        if data is None:
            return None
        if sz == 8:
            fmt = ">Q" if self._get_endian() else "<Q"
        else:
            fmt = ">I" if self._get_endian() else "<I"
        return struct.unpack(fmt, data)[0]

    def read_string(self, addr, max_len=64):
        try:
            inf = gdb.selected_inferior()
            mem = inf.read_memory(addr, max_len)
            raw = bytes(mem)
            nul = raw.find(b'\x00')
            if nul >= 0:
                raw = raw[:nul]
            return raw.decode("utf-8", errors="replace") if raw else None
        except gdb.MemoryError:
            return None

    def get_symbol_addr(self, name):
        try:
            sym, _ = gdb.lookup_symbol(name)
            if sym is not None:
                return int(sym.value().address)
        except (gdb.error, RuntimeError):
            pass
        # Fallback: try as global symbol
        try:
            sym = gdb.lookup_global_symbol(name)
            if sym is not None:
                return int(sym.value().address)
        except (gdb.error, RuntimeError):
            pass
        # Last resort: parse_and_eval
        try:
            return int(gdb.parse_and_eval(f"&{name}"))
        except gdb.error:
            return None

    def get_member_offset(self, type_name, member):
        try:
            t = gdb.lookup_type(type_name)
        except gdb.error:
            # Try with struct prefix
            try:
                t = gdb.lookup_type(f"struct {type_name}")
            except gdb.error:
                return None
        return _find_field_offset_gdb(t, member, 0)

    def get_type_size(self, type_name):
        try:
            t = gdb.lookup_type(type_name)
            return t.sizeof
        except gdb.error:
            try:
                t = gdb.lookup_type(f"struct {type_name}")
                return t.sizeof
            except gdb.error:
                return None

    def eval_expr(self, expr):
        try:
            val = gdb.parse_and_eval(f"&({expr})")
            return int(val)
        except gdb.error:
            return None

    def resolve_symbol(self, addr):
        try:
            output = gdb.execute(f"info symbol {addr}", to_string=True)
            # Output looks like: "func_name in section .text of /path/to/elf"
            # or "No symbol matches ..."
            if output.startswith("No symbol"):
                return None
            # Extract just the symbol name (first token before ' ')
            return output.split()[0]
        except gdb.error:
            return None

    def is_big_endian(self):
        return self._get_endian()


def _find_field_offset_gdb(gdb_type, member_name, base_offset):
    """Recursively search *gdb_type* for a field named *member_name*.

    Anonymous unions/structs (name is None or '') are descended into
    automatically, accumulating their offset.
    """
    # Strip typedefs
    gdb_type = gdb_type.strip_typedefs()

    for field in gdb_type.fields():
        fname = field.name
        if fname == member_name:
            return base_offset + (field.bitpos // 8)
        # Recurse into anonymous aggregates
        if (fname is None or fname == '') and field.type.fields():
            result = _find_field_offset_gdb(
                field.type, member_name,
                base_offset + (field.bitpos // 8))
            if result is not None:
                return result
    return None


# ---------------------------------------------------------------------------
# GDB command class
# ---------------------------------------------------------------------------

class ZephyrCommand(gdb.Command):
    """Zephyr RTOS debugging commands.

    Subcommands:
      threads  - List all threads
      current  - Show current thread details
      readyq   - Show the ready queue
      mutex    - Inspect a k_mutex
      sem      - Inspect a k_sem
      waitq    - Show threads on a wait queue
    """

    def __init__(self):
        super().__init__("zephyr", gdb.COMMAND_USER, prefix=True)

    def invoke(self, arg, from_tty):
        parts = arg.strip().split(None, 1)
        subcmd = parts[0] if parts else ""
        rest = parts[1] if len(parts) > 1 else ""

        backend = GdbBackend()

        dispatch = {
            "threads": lambda: zephyr_core.cmd_threads(backend),
            "current": lambda: zephyr_core.cmd_current(backend),
            "readyq": lambda: zephyr_core.cmd_readyq(backend),
            "mutex": lambda: zephyr_core.cmd_mutex(backend, rest),
            "sem": lambda: zephyr_core.cmd_sem(backend, rest),
            "waitq": lambda: zephyr_core.cmd_waitq(backend, rest),
        }

        if subcmd in dispatch:
            for line in dispatch[subcmd]():
                gdb.write(line + "\n")
        else:
            for line in zephyr_core.cmd_help():
                gdb.write(line + "\n")
            if subcmd:
                gdb.write(f"\nUnknown subcommand: '{subcmd}'\n")


# ---------------------------------------------------------------------------
# Plugin registration
# ---------------------------------------------------------------------------

ZephyrCommand()
print("Zephyr GDB plugin loaded. Type 'zephyr' for help.")
