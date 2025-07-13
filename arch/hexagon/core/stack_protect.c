/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon hardware stack protection implementation
 *
 * Uses the Hexagon FRAMELIMIT register (c16) to detect stack overflows.
 * The FRAMELIMIT register defines the lowest valid stack address for
 * the current thread. Any stack access below this limit triggers a
 * hardware exception.
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <hexagon_arch.h>
#include <kernel_internal.h>

#ifdef CONFIG_HW_STACK_PROTECTION

/* Stack guard size - FRAMELIMIT provides hardware bounds checking */
#define STACK_GUARD_SIZE 64

/* Global framelimit storage for testing */
static uint32_t current_framelimit;

/* Set up stack protection using FRAMELIMIT register */
void z_arch_stack_protection_setup(struct k_thread *thread)
{
	if (thread == NULL) {
		return;
	}

	/* Set FRAMELIMIT to a safe stack limit */
	/* Use architectural interface to get current SP and set limit below it */
	uintptr_t current_sp = hexagon_get_stack_pointer();

	/* Set limit well below current stack pointer */
	uintptr_t stack_limit = current_sp - 8192; /* Leave 8KB buffer */

	/* Store current FRAMELIMIT (using global for now) */
	current_framelimit = stack_limit;

	/* Set the hardware stack limit using architectural interface */
	hexagon_set_framelimit(stack_limit);

	/* Mark stack protection as enabled */
	thread->arch.flags |= 0x04;
}

/* Disable stack protection */
void z_arch_stack_protection_disable(struct k_thread *thread)
{
	if (thread == NULL) {
		return;
	}

	/* Clear the hardware stack limit by setting to 0 */
	hexagon_set_framelimit(0);

	/* Clear protection enabled flag */
	thread->arch.flags &= ~0x04;
}

/* Legacy functions for compatibility */
int z_hexagon_stack_guard_enable(struct k_thread *thread)
{
	z_arch_stack_protection_setup(thread);
	return 0;
}

void z_hexagon_stack_guard_disable(struct k_thread *thread)
{
	z_arch_stack_protection_disable(thread);
}

/* Stack overflow exception handler */
void z_hexagon_stack_overflow_handler(uintptr_t fault_addr, struct arch_esf *esf)
{
	struct k_thread *thread = k_current_get();
	/* For stack overflow detection, use FRAMELIMIT-based approach */
	uintptr_t stack_limit = current_framelimit;
	uintptr_t guard_start = stack_limit - STACK_GUARD_SIZE;

	/* Check if fault is in guard region */
	if (fault_addr >= guard_start && fault_addr < stack_limit) {

		printk("Stack overflow detected!\n");
		printk("Thread: %p, fault address: 0x%08x\n", thread, (uint32_t)fault_addr);
		printk("Stack limit: 0x%08x, guard start: 0x%08x\n", (uint32_t)stack_limit,
		       (uint32_t)guard_start);

		/* Fatal error - stack overflow */
		z_fatal_error(K_ERR_STACK_CHK_FAIL, esf);
	}
}

/* Initialize stack protection */
static int hexagon_stack_protect_init(void)
{
	/* Enable stack protection for main thread */
	z_arch_stack_protection_setup(&z_main_thread);

	printk("Hexagon FRAMELIMIT stack protection initialized\n");
	return 0;
}

/* Context switch hook to update FRAMELIMIT */
void z_arch_stack_protection_switch(struct k_thread *old_thread, struct k_thread *new_thread)
{
#ifdef CONFIG_HW_STACK_PROTECTION
	if (new_thread != NULL && (new_thread->arch.flags & 0x04)) {
		/* Set FRAMELIMIT for the new thread */
		hexagon_set_framelimit(current_framelimit);
	} else {
		/* Disable protection if new thread doesn't have it enabled */
		hexagon_set_framelimit(0);
	}
#endif
}

SYS_INIT(hexagon_stack_protect_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif /* CONFIG_HW_STACK_PROTECTION */
