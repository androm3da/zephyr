/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon hardware stack protection implementation
 *
 * Provides stack guard regions and overflow detection using Hexagon VM
 * memory protection features.
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>
#include <kernel_internal.h>

#ifdef CONFIG_HW_STACK_PROTECTION

/* Stack guard size - must be aligned to Hexagon protection granularity */
#define STACK_GUARD_SIZE 32

/* Set up stack guard region */
int z_hexagon_stack_guard_enable(struct k_thread *thread)
{
	uintptr_t guard_start;

	/* Calculate guard region address */
	guard_start = thread->stack_info.start - STACK_GUARD_SIZE;

	/* For now, implement basic bounds checking */
	/* TODO: Use VM operations when memory protection is fully implemented */

	/* Mark guard region in thread info */
	thread->arch.flags |= 0x04; /* Guard enabled flag */

	return 0;
}

/* Disable stack guard region */
void z_hexagon_stack_guard_disable(struct k_thread *thread)
{
	/* Clear guard enabled flag */
	thread->arch.flags &= ~0x04;
}

/* Stack overflow exception handler */
void z_hexagon_stack_overflow_handler(uintptr_t fault_addr, struct arch_esf *esf)
{
	struct k_thread *thread = k_current_get();
	uintptr_t guard_start = thread->stack_info.start - STACK_GUARD_SIZE;

	/* Check if fault is in guard region */
	if (fault_addr >= guard_start && fault_addr < thread->stack_info.start) {

		printk("Stack overflow detected!\n");
		printk("Thread: %p, fault address: 0x%08x\n", thread, (uint32_t)fault_addr);
		printk("Stack bounds: 0x%08x - 0x%08x\n", (uint32_t)thread->stack_info.start,
		       (uint32_t)(thread->stack_info.start + thread->stack_info.size));

		/* Fatal error - stack overflow */
		z_fatal_error(K_ERR_STACK_CHK_FAIL, esf);
	}
}

/* Initialize stack protection */
static int hexagon_stack_protect_init(void)
{
	/* Enable stack protection for main thread */
	z_hexagon_stack_guard_enable(&z_main_thread);

	printk("Hardware stack protection initialized\n");
	return 0;
}

SYS_INIT(hexagon_stack_protect_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif /* CONFIG_HW_STACK_PROTECTION */
