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
void arch_user_mode_enter(k_thread_entry_t user_entry, void *p1, void *p2, void *p3)
{
	struct k_thread *thread = k_current_get();

	uintptr_t user_sp = thread->stack_info.start + thread->stack_info.size;

	user_sp = ROUND_DOWN(user_sp, ARCH_STACK_PTR_ALIGN);

	memset((void *)thread->stack_info.start, 0, user_sp - thread->stack_info.start);

	thread->arch.priv_level = 1;

	/*
	 * Set the global flag now so that arch_is_user_context() returns
	 * true immediately after vmrte, before the first trap0 fires.
	 * z_hexagon_user_mode_sync() will keep it in sync on every
	 * subsequent kernel re-entry.
	 */
	_hexagon_user_mode_active = 1;

	__asm__ volatile(
		"r4 = %[entry]\n\t"
		"r5 = %[vmest]\n\t"
		"r6 = %[stack]\n\t"
		"r7 = #0\n\t"
		"g0 = r4\n\t"             /* GELR */
		"g1 = r5\n\t"             /* GSR */
		"g2 = r6\n\t"             /* GOSP */
		"g3 = r7\n\t"             /* GBADVA */
		"r0 = %[p1]\n\t"
		"r1 = %[p2]\n\t"
		"r2 = %[p3]\n\t"
		"r29 = %[stack]\n\t"
		"trap1(#1)\n\t"           /* vmrte */
		:
		: [entry] "r"((uintptr_t)user_entry),
		  [vmest] "r"((uint32_t)0xC0000000), /* User mode + IE (GSR bits 31,30) */
		  [stack] "r"(user_sp),
		  [p1] "r"(p1),
		  [p2] "r"(p2),
		  [p3] "r"(p3)
		: "r0", "r1", "r2", "r4", "r5", "r6", "r7", "r29", "memory"
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
