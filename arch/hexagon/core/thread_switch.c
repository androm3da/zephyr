/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon thread switching helpers
 *
 * Provides C wrappers and hooks for thread switching to handle
 * floating-point context and TLS switching.
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>

/* External declarations */
extern void arch_switch(void *switch_to, void **switched_from);

extern void hexagon_tls_switch(struct k_thread *old_thread, struct k_thread *new_thread);

/**
 * Enhanced context switch with TLS support
 * @param new_thread Thread to switch to
 * @param old_thread Thread switching from
 */
void hexagon_context_switch(struct k_thread *new_thread, struct k_thread *old_thread)
{
	/* Switch TLS context */
	hexagon_tls_switch(old_thread, new_thread);

	/* Perform the actual context switch */
	arch_switch(new_thread->switch_handle, &old_thread->switch_handle);
}

/**
 * Thread switch hook called before context switch
 * @param new_thread Thread being switched to
 * @param old_thread Thread being switched from
 */
void arch_switch_hook(struct k_thread *new_thread, struct k_thread *old_thread)
{
	/* Check if old thread is being aborted */
	if (old_thread != NULL && (old_thread->arch.flags & HEXAGON_THREAD_FLAG_ABORT)) {
		/* Handle thread abort cleanup */
		if (old_thread->arch.hw_thread_id >= 0) {
			/* Stop hardware thread if needed */
			__asm__ volatile("trap1(#0x20)" ::: "memory"); /* vmstop */
		}
	}

	/* Switch TLS context */
	hexagon_tls_switch(old_thread, new_thread);
}

/**
 * Thread switch hook called after context switch
 * @param new_thread Thread that was switched to
 * @param old_thread Thread that was switched from
 */
void arch_switch_hook_post(struct k_thread *new_thread, struct k_thread *old_thread)
{
}
