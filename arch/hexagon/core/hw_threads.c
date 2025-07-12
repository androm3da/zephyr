/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon Hardware Thread Support
 *
 * Provides support for multiple hardware threads using Hexagon VM operations.
 * Hexagon processors can support multiple virtual processors (VPs) which
 * appear as hardware threads to the software.
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>

/* Maximum number of hardware threads supported */
#define HEXAGON_MAX_HW_THREADS 4

/* Hardware thread management */
static struct {
	bool enabled;
	int num_threads;
	struct k_thread *hw_threads[HEXAGON_MAX_HW_THREADS];
} hw_thread_mgr = {.enabled = false,
		   .num_threads = 1, /* At least one thread (main) */
		   .hw_threads = {NULL}};

/**
 * Initialize hardware thread support
 * @return 0 on success, negative error code on failure
 */
int hexagon_hw_threads_init(void)
{
	uint32_t num_threads;

	/* Query number of available hardware threads using VM operation */
	__asm__ volatile("r0 = #0x30\n\t"   /* vmgetcfg operation */
			 "trap1(#0x1F)\n\t" /* vmgetcfg */
			 "%0 = r0"
			 : "=r"(num_threads)
			 :
			 : "r0", "memory");

	/* Extract number of threads from configuration */
	hw_thread_mgr.num_threads = (num_threads & 0xF) + 1;

	if (hw_thread_mgr.num_threads > HEXAGON_MAX_HW_THREADS) {
		hw_thread_mgr.num_threads = HEXAGON_MAX_HW_THREADS;
	}

	hw_thread_mgr.enabled = (hw_thread_mgr.num_threads > 1);

	printk("Hexagon: %d hardware threads available\n", hw_thread_mgr.num_threads);

	return 0;
}

/**
 * Get number of available hardware threads
 * @return Number of hardware threads
 */
int hexagon_hw_threads_count(void)
{
	return hw_thread_mgr.num_threads;
}

/**
 * Check if hardware thread support is enabled
 * @return true if enabled, false otherwise
 */
bool hexagon_hw_threads_enabled(void)
{
	return hw_thread_mgr.enabled;
}

/**
 * Assign a thread to run on a specific hardware thread
 * @param thread Thread to assign
 * @param hw_thread_id Hardware thread ID (0-3)
 * @return 0 on success, negative error code on failure
 */
int hexagon_thread_assign_hw(struct k_thread *thread, int hw_thread_id)
{
	if (!hw_thread_mgr.enabled) {
		return -ENOTSUP;
	}

	if (hw_thread_id < 0 || hw_thread_id >= hw_thread_mgr.num_threads) {
		return -EINVAL;
	}

	if (hw_thread_mgr.hw_threads[hw_thread_id] != NULL) {
		return -EBUSY;
	}

	/* Assign thread to hardware thread */
	thread->arch.hw_thread_id = hw_thread_id;
	hw_thread_mgr.hw_threads[hw_thread_id] = thread;

	return 0;
}

/**
 * Remove thread assignment from hardware thread
 * @param thread Thread to unassign
 */
void hexagon_thread_unassign_hw(struct k_thread *thread)
{
	int hw_thread_id = thread->arch.hw_thread_id;

	if (hw_thread_id >= 0 && hw_thread_id < HEXAGON_MAX_HW_THREADS) {
		hw_thread_mgr.hw_threads[hw_thread_id] = NULL;
	}

	thread->arch.hw_thread_id = -1;
}

/**
 * Start execution on a hardware thread
 * @param hw_thread_id Hardware thread ID
 * @param entry_point Entry point for the thread
 * @param stack_ptr Stack pointer for the thread
 * @return 0 on success, negative error code on failure
 */
int hexagon_hw_thread_start(int hw_thread_id, void *entry_point, void *stack_ptr)
{
	if (!hw_thread_mgr.enabled) {
		return -ENOTSUP;
	}

	if (hw_thread_id < 0 || hw_thread_id >= hw_thread_mgr.num_threads) {
		return -EINVAL;
	}

	/* Use VM operation to start hardware thread */
	__asm__ volatile("r0 = %0\n\t"  /* Hardware thread ID */
			 "r1 = %1\n\t"  /* Entry point */
			 "r2 = %2\n\t"  /* Stack pointer */
			 "trap1(#0x21)" /* vmstart */
			 :
			 : "r"(hw_thread_id), "r"(entry_point), "r"(stack_ptr)
			 : "r0", "r1", "r2", "memory");

	return 0;
}

/**
 * Stop execution on a hardware thread
 * @param hw_thread_id Hardware thread ID
 * @return 0 on success, negative error code on failure
 */
int hexagon_hw_thread_stop(int hw_thread_id)
{
	if (!hw_thread_mgr.enabled) {
		return -ENOTSUP;
	}

	if (hw_thread_id < 0 || hw_thread_id >= hw_thread_mgr.num_threads) {
		return -EINVAL;
	}

	/* Use VM operation to stop hardware thread */
	__asm__ volatile("r0 = %0\n\t"  /* Hardware thread ID */
			 "trap1(#0x22)" /* vmstop */
			 :
			 : "r"(hw_thread_id)
			 : "r0", "memory");

	return 0;
}

/**
 * Get current hardware thread ID
 * @return Hardware thread ID, or -1 if not running on hardware thread
 */
int hexagon_get_current_hw_thread(void)
{
	struct k_thread *current = k_current_get();
	return current->arch.hw_thread_id;
}

/**
 * Synchronize all hardware threads
 */
void hexagon_hw_threads_sync(void)
{
	if (hw_thread_mgr.enabled) {
		/* Use VM operation to synchronize all threads */
		__asm__ volatile("trap1(#0x23)" ::: "memory"); /* vmsync */
	}
}

/**
 * Hardware thread barrier - wait for all threads to reach this point
 */
void hexagon_hw_thread_barrier(void)
{
	if (hw_thread_mgr.enabled) {
		/* Use VM operation for thread barrier */
		__asm__ volatile("trap1(#0x24)" ::: "memory"); /* vmbarrier */
	}
}
