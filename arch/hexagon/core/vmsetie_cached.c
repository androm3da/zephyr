/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Cached VM interrupt enable/disable operations
 *
 * Implementation of the cached vmsetie operations from vm_ops.S
 * converted to C with inline assembly for performance-critical paths.
 */

#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <hexagon_vm.h>

/* Global interrupt enable cache */
static volatile int32_t vm_ie_cache;
static volatile bool ie_cache_loaded;

/**
 * Load the interrupt enable cache from current hardware state
 */
void load_ie_cache(void)
{
	vm_ie_cache = hexagon_vm_getie();
	ie_cache_loaded = true;
}

/**
 * Get cached interrupt enable state
 */
int32_t vmgetie_cached(void)
{
	if (!ie_cache_loaded) {
		load_ie_cache();
	}
	return vm_ie_cache;
}

/**
 * Set interrupt enable state with caching
 */
int32_t vmsetie_cached(int32_t new_ie)
{
	return hexagon_vm_setie_cached(new_ie, (int32_t *)&vm_ie_cache);
}

/**
 * Complex cached interrupt enable/disable operation
 * This is a simplified C version of the original assembly implementation
 */
int32_t hexagon_vm_setie_cached(int32_t new_ie, int32_t *cached_ie_ptr)
{
	int32_t old_ie;
	unsigned int key;

	if (!cached_ie_ptr) {
		/* No cache pointer, use direct VM call */
		old_ie = hexagon_vm_getie();
		hexagon_vm_setie(new_ie);
		return old_ie;
	}

	/* Disable interrupts for atomic operation */
	key = arch_irq_lock();

	/* Get current cached value */
	old_ie = *cached_ie_ptr;

	/* Only call VM if value is changing */
	if (old_ie != new_ie) {
		/* Update hardware state */
		hexagon_vm_setie(new_ie);

		/* Update cache */
		*cached_ie_ptr = new_ie;
	}

	/* Restore interrupt state */
	arch_irq_unlock(key);

	return old_ie;
}

/**
 * Clear the interrupt enable cache
 */
void clear_ie_cached(void)
{
	ie_cache_loaded = false;
	vm_ie_cache = 0;
}
