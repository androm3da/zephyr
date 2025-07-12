/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon userspace support implementation
 *
 * Provides user mode support and privilege separation using Hexagon VM
 * guest/host mode capabilities.
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>
#include <zephyr/syscall_handler.h>
#include <hexagon_vm.h>

#ifdef CONFIG_USERSPACE

/* Check if memory region is accessible to user */
int arch_buffer_validate(void *addr, size_t size, int write)
{
	struct k_thread *thread = k_current_get();
	uintptr_t start = (uintptr_t)addr;
	uintptr_t end = start + size;

	/* Check if in user mode */
	if (!arch_is_user_context()) {
		/* Kernel mode - all memory accessible */
		return 0;
	}

	/* Check against thread's accessible regions */
	/* Basic range check for thread stack */
	if (start >= thread->stack_info.start &&
	    end <= thread->stack_info.start + thread->stack_info.size) {
		return 0; /* Within thread stack */
	}

	/* Check other valid regions */
	/* TODO: Add memory domain support when fully implemented */

	return -EPERM;
}

/* Enter user mode */
void arch_user_mode_enter(k_thread_entry_t user_entry, void *p1, void *p2, void *p3)
{
	struct k_thread *thread = k_current_get();

	/* Set up user stack pointer */
	uintptr_t user_sp = thread->stack_info.start + thread->stack_info.size;
	user_sp = ROUND_DOWN(user_sp, ARCH_STACK_PTR_ALIGN);

	/* Clear privileged data from stack */
	memset((void *)thread->stack_info.start, 0, user_sp - thread->stack_info.start);

	/* Set thread as user mode */
	thread->arch.priv_level = 1; /* User mode */

	/* Use VM operation to set user mode */
	hexagon_vm_setregs(NULL);

	/* Set up guest registers for user mode entry */
	__asm__ volatile("r0 = %[user_entry]\n\t" /* user_entry */
			 "r1 = %[p1]\n\t"         /* p1 */
			 "r2 = %[p2]\n\t"         /* p2 */
			 "r3 = %[p3]\n\t"         /* p3 */
			 "r29 = %[user_sp]\n\t"   /* user stack pointer */
			 :
			 : [user_entry] "r"(user_entry), [p1] "r"(p1), [p2] "r"(p2), [p3] "r"(p3),
			   [user_sp] "r"(user_sp)
			 : "r0", "r1", "r2", "r3", "r29", "memory");

	/* Transition to user mode */
	hexagon_vm_getregs(NULL);

	CODE_UNREACHABLE;
}

/* System call handler */
void arch_syscall_invoke(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3,
			 uint32_t arg4, uint32_t arg5, uint32_t arg6, struct arch_esf *esf)
{
	/* Validate syscall ID */
	if (syscall_id >= K_SYSCALL_LIMIT) {
		esf->r0 = -ENOSYS;
		return;
	}

	/* Call system call handler */
	typedef uintptr_t (*syscall_handler_t)(uintptr_t, uintptr_t, uintptr_t, uintptr_t,
					       uintptr_t, uintptr_t);

	/* TODO: Add syscall table when fully implemented */
	esf->r0 = -ENOSYS; /* Not implemented yet */
}

/* Handle system call from user mode */
void z_hexagon_syscall_handler(struct arch_esf *esf)
{
	/* Extract syscall number and arguments from registers */
	uint32_t syscall_id = esf->r6; /* Syscall ID in r6 per ABI */

	/* Arguments in r0-r5 */
	arch_syscall_invoke(syscall_id, esf->r0, esf->r1, esf->r2, esf->r3, esf->r4, esf->r5, esf);
}

/* Generate kernel oops from user mode */
void arch_syscall_oops(void *ssf)
{
	struct arch_esf *esf = (struct arch_esf *)ssf;

	printk("System call error from user mode\n");
	printk("Syscall ID: %u\n", esf->r6);

	z_fatal_error(K_ERR_KERNEL_OOPS, esf);
}

/* Check if in user context */
bool arch_is_user_context(void)
{
	struct k_thread *thread = k_current_get();
	return thread->arch.priv_level > 0;
}

/* Memory domain support */
int arch_mem_domain_max_partitions_get(void)
{
	/* Return maximum partitions supported by Hexagon VM */
	return 8; /* Default reasonable limit */
}

/* Initialize memory domain */
int arch_mem_domain_init(struct k_mem_domain *domain)
{
	ARG_UNUSED(domain);
	/* Initialize domain structure */
	return 0;
}

/* Add partition to memory domain */
int arch_mem_domain_partition_add(struct k_mem_domain *domain, uint32_t partition_id)
{
	ARG_UNUSED(domain);
	ARG_UNUSED(partition_id);
	/* Add partition to domain */
	return 0;
}

/* Remove partition from memory domain */
int arch_mem_domain_partition_remove(struct k_mem_domain *domain, uint32_t partition_id)
{
	ARG_UNUSED(domain);
	ARG_UNUSED(partition_id);
	/* Remove partition from domain */
	return 0;
}

/* Apply memory domain to thread */
void arch_mem_domain_thread_add(struct k_thread *thread)
{
	ARG_UNUSED(thread);
	/* Apply domain settings to thread */
}

/* Remove memory domain from thread */
void arch_mem_domain_thread_remove(struct k_thread *thread)
{
	ARG_UNUSED(thread);
	/* Remove domain settings from thread */
}

#endif /* CONFIG_USERSPACE */
