# Hexagon userspace bring-up: bug fixes and open issue (2026-09-10)

Work on `bcain/hexagon_support_fixes`, rebased onto `bcain/hexagon_support`,
porting `CONFIG_USERSPACE` support from `bcain/hexagon_latest` and getting
`kernel.threads.thread_stack` running on `qemu_hexagon`. Three commits landed
this session; one bug remains open.

## Commits

1. `ci: drop the hexagon workflow in favor of board only_tags` -- replaces
   `.github/workflows/hexagon.yaml` with `only_tags`/`ignore_tags` in
   `boards/qemu/hexagon/qemu_hexagon.yaml`. Verified with a full twister run
   against the real Hexagon LLVM toolchain and qemu-system-hexagon: 180
   passed, 2 built-only (UART-dependent, cannot run headless), 0
   failed/errored.
2. `hexagon: select ARCH_HAS_DIRECT_INTERRUPTS` -- the arch defines
   `ARCH_IRQ_DIRECT_CONNECT()`/`ARCH_ISR_DIRECT_DECLARE()` but never selected
   the Kconfig symbol that gates their use.
3. `hexagon: add userspace, syscall and MMU support` -- the userspace port
   itself: trap0 syscall dispatch, vmrte-based privilege drop, stack
   protection, and MMU stubs. Four separate bugs found and fixed during
   bring-up are described below.

## Fixes found and applied

### 1. `arch_mem_map()`/`arch_mem_unmap()` corrupting unrelated translations

The initial port maintained its own PGD/L2 page table structure, built and
activated via `vmnewmap`, disjoint from the boot-time identity map that
`hvm_event_vectors.S` builds and activates before `z_prep_c()` runs. Calling
`arch_mem_unmap()` invoked `vmclrmap()` against addresses whose only
"mapping" lived in this second, unused table -- corrupting TLB/STLB state
for the boot table that was actually active, breaking unrelated code pages.

**Fix**: deleted the PGD/L2 machinery. `arch_mem_map()`/`arch_mem_unmap()`
are now no-ops (asserting `virt == phys`) and `arch_page_phys_get()` is a
trivial identity. `CONFIG_KERNEL_DIRECT_MAP` is selected so every
`device_map()`/`k_mem_map_phys_bare()` caller resolves `virt == phys`,
already covered by the boot identity map.

### 2. Boot identity map missing the user-access bit

The boot-time PDE for the main RAM/SRAM 4MB superpage
(`hvm_event_vectors.S:_setup_page_table`) never set `__HVM_PTE_U`, so any
user-mode access anywhere in RAM faulted immediately with `PRIV_NO_READ`
(cause `0x22`).

Confirmed against the real H2 hardware PTE layout
(`hexagon-hypervisor/libs/h2/common/h2_common_pagewalk.h`): bit 5 is `u`,
matching Zephyr's `__HVM_PTE_U = (1 << 5)`. Traced the actual permission
check through H2's `H2K_mem_pagewalk_l1()`/`H2K_pagewalk_update_translation()`
(`in.xwru &= (pte.xwr << 1) | pte.u`) and `H2K_mem_tlbfmt_from_trans()`
(rejects a translation whose X/W/R bits are all clear) -- confirms this is
the only permission-bit dependency at TLB-fill time and that the fix is
correctly targeted.

**Fix**: added `__HVM_PTE_U` to that PDE's flags. UART MMIO is intentionally
left supervisor-only.

### 3. Stale TLS pointer across `arch_tls_stack_setup()`'s second call

`k_thread_user_mode_enter()` calls `arch_tls_stack_setup()` again right
before `arch_user_mode_enter()`, laying out a fresh TLS block and recopying
the whole `.tdata`/`.tbss` template into it -- which zeroes `z_tls_current`
along with the rest of `.tbss`. `ugp` is not reloaded from the new block
until the `vmrte` at the end of `arch_user_mode_enter()`, and `vmrte` itself
never touches `ugp` (confirmed against the HVM spec: the event record it
restores is GELR/GSR/GOSP/GBADVA only). Any `k_current_get()` call in
between -- including the one at the top of `arch_user_mode_enter()` itself
-- read back a NULL thread pointer.

**Fix**: use `k_sched_current_thread_query()` instead of `k_current_get()`
at function entry, and explicitly restore `z_tls_current = thread` right
before the `vmrte`, since `ugp` keeps pointing at the same (now correctly
populated) block afterwards.

### 4. `memset()` zeroing the live call chain it is running on

`arch_user_mode_enter()` zeroed the entire user stack buffer
(`[stack_info.start, user_sp)`) before dropping to user mode, following the
pattern other arches use. Those arches run this from a separate privileged
stack; Hexagon runs `k_thread_user_mode_enter()` -> `arch_user_mode_enter()`
directly on the thread's own stack, so this zeroed out the live return
addresses of the C call chain currently executing on that same stack,
including its own.

Bisected the safe boundary empirically:
- Up to `kernel_sp` (`__builtin_frame_address(0)`, i.e. FP after
  `allocframe`) was still unsafe: this frame's own locals/spills live in
  `[SP, FP)`, below FP.
- Up to the raw current SP (`r29`) was also unsafe: `memset()` itself opens
  an 8-byte leaf frame (its own FP:LR save slot) at `[SP-8, SP)` on entry,
  so the last store of a fill reaching all the way to SP overwrites that
  slot, and `memset()` returns to the zero it just wrote.

**Fix**: read live `r29` via inline asm and zero
`[stack_info.start, r29 - 8)`, leaving `memset()`'s own frame alone.

### 5. `user_sp` not accounting for the TLS block reservation

Even with (3) and (4) fixed, `arch_user_mode_enter()` computed
`user_sp = stack_info.start + stack_info.size` -- the raw top of the stack
buffer, ignoring the TLS block that the second `arch_tls_stack_setup()` call
had just written directly below that same top address. The user thread's
first few pushes on entry immediately overwrote its own TLS block,
including the just-restored `z_tls_current` (fix 3), reproducing the same
"`k_current_get()` returns NULL" symptom one syscall later, from inside
`z_impl_stack_info_get()`.

RISC-V's `arch_user_mode_enter()` was the reference for the right quantity:
`stack_info.delta` is documented (`struct _stack_info` in
`include/zephyr/kernel/thread.h`) as exactly what to subtract --
`(start + size - delta)` is "the initial stack pointer for a thread".
`stack_info.delta` is already populated at thread-creation time by the
*first* `arch_tls_stack_setup()` call and stays valid across the second one,
since `z_tls_data_size()` is constant.

**Fix**: `user_sp = stack_info.start + stack_info.size - stack_info.delta`.

### 6. `_hexagon_user_mode_active` reset by unrelated events

`z_syscall_trap()` (`include/zephyr/syscall.h`) decides whether a syscall
wrapper actually traps into the kernel or calls `z_impl_*()` directly, based
on `arch_is_user_context()`, which reads Hexagon's global
`_hexagon_user_mode_active` flag. That flag is re-derived on every event
exit by `z_hexagon_user_mode_sync()`, from `_current->arch.priv_level` --
but `arch_user_mode_enter()` never set `thread->arch.priv_level`, so it
stayed 0 forever. The first unrelated event serviced while the user thread
happened to be `_current` (an interrupt, not even one this thread caused)
reset the global flag to 0. Every syscall the thread made after that skipped
the trap0 trampoline and ran the kernel implementation directly, still at
real hardware user privilege -- observed as a `0x24` ("Load User Access
Violation") fault in `pl011_poll_out()` when `printk()` tried to touch UART
MMIO from actual `MMU_USER_IDX` privilege (confirmed via
`qemu-system-hexagon -d mmu` trace).

**Fix**: set `thread->arch.priv_level = 1` in `arch_user_mode_enter()`
alongside the `_hexagon_user_mode_active = 1` it already sets.

## Open issue: not yet resolved

With fixes 1-6 applied, `kernel.threads.thread_stack`'s first user-mode
subtest (`stest_thread_launch(K_USER | K_INHERIT_PERMS, false)`, the "direct
launch" case) gets further than before -- past `arch_user_mode_enter()`,
into `stack_buffer_scenarios()` -- but still fails. The failure mode changed
across each fix (from `k_current_get()==NULL` at the first syscall, to a
`0x24` UART fault, to the current state): after printing
`" - Testing user mode (direct launch)"`, a burst of garbage bytes appears
on the serial console, followed by `cause=0x22 pc=0x0` -- a jump to a
zeroed return address, likely from a return-address slot on the stack
getting clobbered by something. Not yet root-caused; the debugging session
was stopped here per explicit instruction rather than continuing to chase
it. `only_tags` in `qemu_hexagon.yaml` does not yet include this or other
userspace-tagged tests.

Next steps for whoever picks this up:
- Get a clean reproduction with `-d guest_errors,int,mmu,in_asm` and
  `-dfilter` narrowed to the fault PC to see exactly what wrote the garbage
  and what the zeroed return address slot's expected value should have
  been.
- Check whether the garbage bytes on serial are meaningful (e.g. a
  legitimate `printk` fragment interrupted mid-write, or literal stack
  contents leaking to the UART) -- that would narrow down which buffer got
  aliased with which stack slot.
- Re-check the interaction between `EVENT_ENTRY`'s `allocframe` (which
  lands on `r29`/GOSP, the *kernel* stack saved in `arch_user_mode_enter()`)
  and any nested event taken while still early in user-mode execution, since
  fix 6 was about exactly this kind of cross-thread/cross-event state
  bleeding through a global flag.
