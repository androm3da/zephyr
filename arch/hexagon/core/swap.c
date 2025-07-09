/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>

/**
 * @brief Swap to another thread.
 *
 * This function performs a context switch to another thread.
 * For Hexagon VM, this will be a simplified implementation
 * for initial porting.
 *
 * @param key Interrupt key from arch_irq_lock()
 * @return The key passed to arch_irq_unlock() when swapped back
 */
unsigned int arch_swap(unsigned int key)
{
	/* TODO: Implement proper context switching for Hexagon VM */
	/* For now, just return the key to maintain interface compatibility */
	return key;
}
