/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Hexagon userspace support
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>
#include <zephyr/internal/syscall_handler.h>
#include <hexagon_vm.h>
#include <offsets_short.h>

#ifdef CONFIG_USERSPACE

int arch_buffer_validate(const void *addr, size_t size, int write)
{
	struct k_thread *thread = k_current_get();
	uintptr_t start = (uintptr_t)addr;
	uintptr_t end = start + size;

	if (!arch_is_user_context()) {
		return 0;
	}

	/* Check thread stack */
	if (start >= thread->stack_info.start &&
	    end <= thread->stack_info.start + thread->stack_info.size) {
		return 0; /* Within thread stack */
	}

	/* Check memory domain partitions */
	if (thread->mem_domain_info.mem_domain != NULL) {
		struct k_mem_domain *domain = thread->mem_domain_info.mem_domain;
		int num_parts = domain->num_partitions;

		for (int i = 0; i < num_parts; i++) {
			const struct k_mem_partition *part = &domain->partitions[i];
			uintptr_t part_start = part->start;
			uintptr_t part_end = part_start + part->size;

			if (start < part_start || end > part_end) {
				continue;
			}

			/* For a write access, the partition must be writable */
			if (write && !K_MEM_PARTITION_IS_WRITABLE(part->attr)) {
				continue;
			}

			return 0; /* Within a valid partition */
		}
	}

	return -EPERM;
}

size_t arch_user_string_nlen(const char *s, size_t maxsize, int *err_arg)
{
	if (arch_buffer_validate(s, maxsize, 0)) {
		*err_arg = -1;
		return 0;
	}

	*err_arg = 0;
	return strnlen(s, maxsize);
}

/*
 * Guest register usage for vmrte:
 *   G0 = GELR (entry point)
 *   G1 = GSR (bit 31 = user mode, bit 30 = IE)
 *   G2 = GOSP (user stack pointer)
 *   G3 = GBADVA (0 for normal entry)
 */
static void __used __naked hexagon_user_thread_exit(void)
{
	/*
	 * User function returned — call k_thread_abort(self) via
	 * explicit trap0 syscall.  We cannot use the C wrapper because
	 * the compiler may optimize away the user-mode check.
	 *
	 * Syscall convention: r0 = arg (thread), r6 = syscall number.
	 * k_current_get() is just a memory read (no privilege needed).
	 */
	__asm__ volatile(
		/* r0 = _kernel.cpus[0].current (k_current_get) */
		"r0 = ##_kernel\n\t"
		"r0 = add(r0, #%[cpus_off])\n\t"
		"r0 = memw(r0+#%[cur_off])\n\t"
		/* syscall: k_thread_abort(r0) */
		"r6 = #%[sc_id]\n\t"
		"trap0(#0x1)\n\t"
		/* should not return — loop as backstop */
		"1: jump 1b\n\t"
		:
		: [cpus_off] "i"(___kernel_t_cpus_OFFSET),
		  [cur_off] "i"(___cpu_t_current_OFFSET),
		  [sc_id] "i"(K_SYSCALL_K_THREAD_ABORT)
		:
	);
}

void arch_user_mode_enter(k_thread_entry_t user_entry, void *p1, void *p2, void *p3)
{
	struct k_thread *thread = k_current_get();

	uintptr_t user_sp = thread->stack_info.start + thread->stack_info.size;

	user_sp = ROUND_DOWN(user_sp, ARCH_STACK_PTR_ALIGN);

	/*
	 * Save the current kernel SP as GOSP.  When H2 delivers an event
	 * from user mode (trap0 syscall), it swaps r29 with GOSP — landing
	 * the kernel event handler on this kernel stack rather than the
	 * user stack.  Without this, EVENT_ENTRY's allocframe overwrites
	 * the user function's saved LR on the user stack.
	 */
	uintptr_t kernel_sp = (uintptr_t)__builtin_frame_address(0);

	memset((void *)thread->stack_info.start, 0, user_sp - thread->stack_info.start);

	thread->arch.priv_level = 1;

	/*
	 * Set the global flag now so that arch_is_user_context() returns
	 * true immediately after vmrte, before the first trap0 fires.
	 * z_hexagon_user_mode_sync() will keep it in sync on every
	 * subsequent kernel re-entry.
	 */
	_hexagon_user_mode_active = 1;

	/*
	 * H2 vmrte with GSSR.UM swaps r29 <-> GOSP.  To end up with
	 * r29=user_sp in user mode, set:
	 *   r29 = kernel_sp (will become gosp after swap)
	 *   GOSP (g2) = user_sp (will become r29 after swap)
	 */
	__asm__ volatile(
		"r4 = %[entry]\n\t"
		"r5 = %[vmest]\n\t"
		"r6 = %[stack]\n\t"      /* GOSP = user SP (becomes r29) */
		"r7 = #0\n\t"
		"g0 = r4\n\t"            /* GELR = user entry */
		"g1 = r5\n\t"            /* GSR = UM + IE */
		"g2 = r6\n\t"            /* GOSP = user SP */
		"g3 = r7\n\t"            /* GBADVA = 0 */
		"r0 = %[p1]\n\t"
		"r1 = %[p2]\n\t"
		"r2 = %[p3]\n\t"
		"r29 = %[ksp]\n\t"       /* kernel SP (becomes gosp) */
		"r31 = %[exit_fn]\n\t"   /* LR = user thread exit stub */
		"r30 = #0\n\t"           /* FP = 0 (no parent frame) */
		"trap1(#1)\n\t"          /* vmrte */
		:
		: [entry] "r"((uintptr_t)user_entry),
		  [vmest] "r"((uint32_t)0xC0000000), /* User mode + IE */
		  [ksp] "r"(kernel_sp),
		  [stack] "r"(user_sp),
		  [p1] "r"(p1),
		  [p2] "r"(p2),
		  [p3] "r"(p3),
		  [exit_fn] "r"((uintptr_t)hexagon_user_thread_exit)
		: "r0", "r1", "r2", "r4", "r5", "r6", "r7",
		  "r29", "r30", "r31", "memory"
	);

	CODE_UNREACHABLE;
}

void arch_syscall_invoke(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3,
			 uint32_t arg4, uint32_t arg5, uint32_t arg6, struct arch_esf *esf)
{
	if (syscall_id >= K_SYSCALL_LIMIT) {
		esf->r0 = -ENOSYS;
		return;
	}

	esf->r0 = (uint32_t)_k_syscall_table[syscall_id](
		arg1, arg2, arg3, arg4, arg5, arg6, esf);
}

void z_hexagon_syscall_handler(struct arch_esf *esf)
{
	uint32_t syscall_id = esf->r6;

	arch_syscall_invoke(syscall_id, esf->r0, esf->r1, esf->r2, esf->r3, esf->r4, esf->r5, esf);
}

FUNC_NORETURN void arch_syscall_oops(void *ssf)
{
	struct arch_esf *esf = (struct arch_esf *)ssf;

	z_fatal_error(K_ERR_KERNEL_OOPS, esf);
	CODE_UNREACHABLE;
}

int arch_mem_domain_max_partitions_get(void)
{
	return 8;
}

int arch_mem_domain_init(struct k_mem_domain *domain)
{
	ARG_UNUSED(domain);
	return 0;
}

int arch_mem_domain_partition_add(struct k_mem_domain *domain, uint32_t partition_id)
{
	ARG_UNUSED(domain);
	ARG_UNUSED(partition_id);
	return 0;
}

int arch_mem_domain_partition_remove(struct k_mem_domain *domain, uint32_t partition_id)
{
	ARG_UNUSED(domain);
	ARG_UNUSED(partition_id);
	return 0;
}

void arch_mem_domain_thread_add(struct k_thread *thread)
{
	ARG_UNUSED(thread);
}

void arch_mem_domain_thread_remove(struct k_thread *thread)
{
	ARG_UNUSED(thread);
}

#endif /* CONFIG_USERSPACE */
