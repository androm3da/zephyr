/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/thread.h>
#include <string.h>
#include <kernel_internal.h>

void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack, char *stack_ptr,
		     k_thread_entry_t entry, void *p1, void *p2, void *p3)
{
	/* Initialize the thread's callee-saved context */
	/* Clear all callee-saved registers to zero */
	memset(&thread->callee_saved, 0, sizeof(thread->callee_saved));

	/* Set up the stack pointer - Hexagon stack grows downward */
	/* Align to 8-byte boundary as required by Hexagon ABI */
	uintptr_t stack_end = (uintptr_t)ROUND_DOWN(stack_ptr, ARCH_STACK_PTR_ALIGN);

	/* Set the initial stack pointer */
	thread->callee_saved.r30_sp = stack_end;

	/* Set the frame pointer to the same value initially */
	thread->callee_saved.r29_fp = stack_end;

	/* Set up the thread entry point in the link register */
	/* When arch_switch returns, it will "return" to this address */
	thread->callee_saved.r31_lr = (uint32_t)z_thread_entry;

	/* Store thread parameters in callee-saved registers */
	/* These will be preserved across the context switch */
	thread->callee_saved.r16 = (uint32_t)entry; /* Thread entry function */
	thread->callee_saved.r17 = (uint32_t)p1;    /* Parameter 1 */
	thread->callee_saved.r18 = (uint32_t)p2;    /* Parameter 2 */
	thread->callee_saved.r19 = (uint32_t)p3;    /* Parameter 3 */

	/* Initialize arch-specific data */
	thread->arch.swap_return_value = 0;
	memset(thread->arch.vm_event_info, 0, sizeof(thread->arch.vm_event_info));

	/* For CONFIG_USE_SWITCH, initialize switch_handle to point to callee_saved context */
	/* New threads will have their switch_handle point to their stack context */
	thread->switch_handle = &thread->callee_saved;
}

char *arch_k_thread_stack_buffer(k_thread_stack_t *stack)
{
	return (char *)stack;
}

size_t arch_k_thread_stack_size(k_thread_stack_t *stack)
{
	return 0; /* Stub */
}
