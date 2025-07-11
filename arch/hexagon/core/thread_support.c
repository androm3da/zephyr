/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <zephyr/arch/hexagon/thread.h>
#include <zephyr/arch/hexagon/vm_ops.h>

/* Get the current CPU */
int arch_curr_cpu(void)
{
	/* Get virtual processor ID using VM operation */
	return hexagon_vm_vpid();
}

/* Check if in user context */
bool arch_is_user_context(void)
{
	struct k_thread *thread = k_current_get();
	return thread->arch.priv_level > 0;
}

/* Architecture-specific thread initialization */
void z_arch_thread_init(struct k_thread *thread)
{
	thread->arch.priv_level = 0; /* Start in supervisor mode */
	thread->arch.flags = 0;
}

/* Return the current thread's stack pointer */
uintptr_t z_arch_get_thread_sp(void)
{
	uintptr_t sp;
	__asm__ volatile("%[sp] = r29" : [sp] "=r"(sp));
	return sp;
}

/* Thread entry wrapper - called when thread first runs */
void z_hexagon_thread_entry(k_thread_entry_t entry, void *p1, void *p2, void *p3)
{
	/* Enable interrupts for new thread using VM operation */
	hexagon_vm_vmsetie(1);

	/* Call the actual thread entry point */
	entry(p1, p2, p3);

	/* Thread completed - terminate */
	k_thread_abort(k_current_get());

	/* Should never reach here */
	CODE_UNREACHABLE;
}
