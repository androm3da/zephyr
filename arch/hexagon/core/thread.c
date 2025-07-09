/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/thread.h>

void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack, char *stack_ptr,
		     k_thread_entry_t entry, void *p1, void *p2, void *p3)
{
	/* Stub implementation */
}

void arch_switch(void *switch_to, void **switched_from)
{
	/* Stub implementation */
}

void arch_thread_return_value_set(struct k_thread *thread, unsigned int value)
{
	thread->arch.swap_return_value = value;
}

char *arch_k_thread_stack_buffer(k_thread_stack_t *stack)
{
	return (char *)stack;
}

size_t arch_k_thread_stack_size(k_thread_stack_t *stack)
{
	return 0; /* Stub */
}

void arch_switch_to_main_thread(struct k_thread *main_thread, k_thread_stack_t *stack,
				k_thread_entry_t _main)
{
	/* Set up the main thread as current thread */
	_current = main_thread;

	/* For now, just call the main function directly */
	/* TODO: Proper context setup for Hexagon VM */
	_main(NULL, NULL, NULL);
}
