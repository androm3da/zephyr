/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/thread.h>
#include <string.h>
#include <kernel_internal.h>

void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack, char *stack_ptr,
		     k_thread_entry_t entry, void *p1, void *p2, void *p3)
{
	/* Initialize the enhanced thread context */
	memset(&thread->arch.context, 0, sizeof(thread->arch.context));

	/* Set up the stack pointer - Hexagon stack grows downward */
	/* Align to 8-byte boundary as required by Hexagon ABI */
	uintptr_t stack_end = (uintptr_t)ROUND_DOWN(stack_ptr, ARCH_STACK_PTR_ALIGN);

	/* Set up initial context using enhanced structure */
	thread->arch.context.r29 = stack_end;                /* Stack pointer */
	thread->arch.context.r30 = (uint32_t)z_thread_entry; /* Link register for thread entry */
	thread->arch.context.r31 = 0;                        /* Frame pointer */

	/* Store thread parameters in callee-saved registers */
	/* These will be preserved across the context switch */
	thread->arch.context.r16 = (uint32_t)entry; /* Thread entry function */
	thread->arch.context.r17 = (uint32_t)p1;    /* Parameter 1 */
	thread->arch.context.r18 = (uint32_t)p2;    /* Parameter 2 */
	thread->arch.context.r19 = (uint32_t)p3;    /* Parameter 3 */

	/* Initialize control registers */
	thread->arch.context.usr = 0; /* User status register */
	thread->arch.context.ugp = 0; /* User global pointer */

	/* Initialize loop registers */
	thread->arch.context.lc0 = 0;
	thread->arch.context.lc1 = 0;
	thread->arch.context.sa0 = 0;
	thread->arch.context.sa1 = 0;

	/* Initialize predicate registers */
	thread->arch.context.p3_0 = 0;

	/* Initialize VM guest registers */
	thread->arch.context.g0 = 0;
	thread->arch.context.g1 = 0;
	thread->arch.context.g2 = 0;
	thread->arch.context.g3 = 0;

	/* Initialize arch-specific data */
	thread->arch.swap_return_value = 0;
	thread->arch.priv_level = 0; /* Supervisor mode initially */
	thread->arch.flags = 0;
	memset(thread->arch.vm_event_info, 0, sizeof(thread->arch.vm_event_info));

	/* TODO: Add enhanced thread features when structure is updated */

	/* Backward compatibility: also initialize callee_saved structure for CONFIG_USE_SWITCH */
	memset(&thread->callee_saved, 0, sizeof(thread->callee_saved));
	thread->callee_saved.r30_sp = stack_end;
	thread->callee_saved.r29_fp = stack_end;
	thread->callee_saved.r31_lr = (uint32_t)z_thread_entry;
	thread->callee_saved.r16 = (uint32_t)entry;
	thread->callee_saved.r17 = (uint32_t)p1;
	thread->callee_saved.r18 = (uint32_t)p2;
	thread->callee_saved.r19 = (uint32_t)p3;

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
