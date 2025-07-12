/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon thread termination and abort functionality
 */

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>
#include <ksched.h>
#include <kswap.h>
#include <zephyr/arch/hexagon/arch.h>
#include <kernel_internal.h>

/* Forward declarations */
static void hexagon_cleanup_thread_resources(struct k_thread *thread);
static void hexagon_abort_from_exception(void);
void arch_thread_abort(struct k_thread *thread);

void z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	/* Enhanced Hexagon thread abort with proper cleanup */
	arch_thread_abort(thread);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}

/* Architecture-specific thread abort */
void arch_thread_abort(struct k_thread *thread)
{
	unsigned int key;

	/* Disable interrupts */
	key = arch_irq_lock();

	/* Handle different abort scenarios */
	if (thread == k_current_get()) {
		/* Self-abort */
		if (_current_cpu->nested > 1) {
			/* In exception handler - special handling needed */
			hexagon_abort_from_exception();
		} else {
			/* Normal thread context */
			z_thread_essential_clear(thread);
			z_thread_monitor_exit(thread);

			/* Clean up thread resources */
			hexagon_cleanup_thread_resources(thread);
			z_thread_abort(thread);

			/* If this is a hardware thread, stop it */
			/* TODO: Add hardware thread stop when fully implemented */

			/* Never returns - switches to next thread */
			z_swap_unlocked();
		}
	} else {
		/* Aborting another thread */
		z_thread_essential_clear(thread);
		z_thread_monitor_exit(thread);

		/* Remove from ready queue if needed */
		/* TODO: Use proper ready queue removal function */

		/* Clean up thread resources */
		hexagon_cleanup_thread_resources(thread);
		z_thread_abort(thread);

		/* Re-enable interrupts */
		arch_irq_unlock(key);
	}
}

/* Clean up Hexagon-specific thread resources */
static void hexagon_cleanup_thread_resources(struct k_thread *thread)
{
	/* TODO: Add TLS cleanup when fully integrated */

	/* Clear any architecture-specific flags */
	thread->arch.flags = 0;
}

/* Abort from exception context */
static void hexagon_abort_from_exception(void)
{
	/* Special handling when aborting from exception */
	struct k_thread *thread = k_current_get();

	/* Mark thread as aborting */
	/* TODO: Use proper abort flag when thread flags are fully implemented */
	thread->arch.flags |= 0x01; /* Simple abort flag */

	/* Return from exception will handle the abort */
}
