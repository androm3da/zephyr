/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "../include/memory.h"
#include <hexagon_vm.h>

/* Cache operations */
int arch_dcache_range(void *addr, size_t size, int op)
{
	return hexagon_vm_cache(op, (uint32_t)addr, size);
}

void arch_dcache_flush_all(void)
{
	/* Flush entire data cache */
	arch_dcache_range(NULL, 0, HEXAGON_CACHE_CLEAN);
}

void arch_dcache_invd_all(void)
{
	/* Invalidate entire data cache */
	arch_dcache_range(NULL, 0, HEXAGON_CACHE_INVALIDATE);
}

void arch_dcache_flush_and_invd_all(void)
{
	/* Clean and invalidate entire data cache */
	arch_dcache_range(NULL, 0, HEXAGON_CACHE_CLEAN_INVALIDATE);
}
