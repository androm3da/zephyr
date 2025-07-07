/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>

/* Hexagon V66 cache management functions */

int arch_dcache_enable(void)
{
	/* Cache is typically enabled by default on Hexagon */
	return 0;
}

int arch_dcache_disable(void)
{
	/* For simulator, we might not need to actually disable cache */
	return 0;
}

int arch_icache_enable(void)
{
	/* Instruction cache is typically enabled by default on Hexagon */
	return 0;
}

int arch_icache_disable(void)
{
	/* For simulator, we might not need to actually disable cache */
	return 0;
}

void arch_dcache_flush_all(void)
{
	/* Flush all data cache */
	/* QEMU simulation - cache operations are no-ops */
	__asm__ volatile ("" ::: "memory");
}

void arch_dcache_invd_all(void)
{
	/* Invalidate all data cache */
	/* QEMU simulation - cache operations are no-ops */
	__asm__ volatile ("" ::: "memory");
}

void arch_dcache_flush_and_invd_all(void)
{
	/* Flush and invalidate all data cache */
	arch_dcache_flush_all();
	arch_dcache_invd_all();
}

void arch_dcache_flush_range(void *addr, size_t size)
{
	/* Flush specific range of data cache */
	/* For now, just flush all - can be optimized later */
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
	arch_dcache_flush_all();
}

void arch_dcache_invd_range(void *addr, size_t size)
{
	/* Invalidate specific range of data cache */
	/* For now, just invalidate all - can be optimized later */
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
	arch_dcache_invd_all();
}

void arch_dcache_flush_and_invd_range(void *addr, size_t size)
{
	/* Flush and invalidate specific range of data cache */
	arch_dcache_flush_range(addr, size);
	arch_dcache_invd_range(addr, size);
}

void arch_icache_flush_all(void)
{
	/* Flush all instruction cache */
	/* QEMU simulation - cache operations are no-ops */
	__asm__ volatile ("" ::: "memory");
}

void arch_icache_invd_all(void)
{
	/* Invalidate all instruction cache */
	/* QEMU simulation - cache operations are no-ops */
	__asm__ volatile ("" ::: "memory");
}

void arch_icache_flush_and_invd_all(void)
{
	/* Flush and invalidate all instruction cache */
	arch_icache_flush_all();
}

void arch_icache_flush_range(void *addr, size_t size)
{
	/* Flush specific range of instruction cache */
	/* For now, just flush all - can be optimized later */
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
	arch_icache_flush_all();
}

void arch_icache_invd_range(void *addr, size_t size)
{
	/* Invalidate specific range of instruction cache */
	/* For now, just invalidate all - can be optimized later */
	ARG_UNUSED(addr);
	ARG_UNUSED(size);
	arch_icache_invd_all();
}

void arch_icache_flush_and_invd_range(void *addr, size_t size)
{
	/* Flush and invalidate specific range of instruction cache */
	arch_icache_flush_range(addr, size);
}

size_t arch_dcache_line_size_get(void)
{
	/* Typical cache line size for Hexagon V66 */
	return 32;
}

size_t arch_icache_line_size_get(void)
{
	/* Typical cache line size for Hexagon V66 */
	return 32;
}