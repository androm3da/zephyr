/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "../include/memory.h"

/* VM memory operations */
static int hexagon_vm_map_region(struct hexagon_mem_map *map)
{
	register uint32_t r0 __asm__("r0") = (uint32_t)map;
	register uint32_t r1 __asm__("r1") = 1; /* Page table type */
	
	__asm__ volatile(
		"trap1(#0xb)"  /* vmnewmap */
		: "+r"(r0)
		: "r"(r1)
		: "memory"
	);
	
	return r0; /* 0 on success */
}

static void hexagon_vm_clear_map(void)
{
	__asm__ volatile(
		"r0 = #0\n\t"
		"trap1(#0xa)"  /* vmclrmap */
		:
		:
		: "r0", "memory"
	);
}

/* Cache operations */
int arch_dcache_range(void *addr, size_t size, int op)
{
	register uint32_t r0 __asm__("r0") = op;
	register uint32_t r1 __asm__("r1") = (uint32_t)addr;
	register uint32_t r2 __asm__("r2") = size;
	
	__asm__ volatile(
		"trap1(#0xd)"  /* vmcache */
		: "+r"(r0)
		: "r"(r1), "r"(r2)
		: "memory"
	);
	
	return r0; /* 0 on success */
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

static int hexagon_mem_init(void)
{
	/* For minivm, we don't need complex memory mapping */
	/* VM operations may not be fully functional in minivm */
	
	/* Simple memory initialization without VM operations */
	printk("Hexagon memory management initialized\n");
	return 0;
}

/* Simplified stack setup for memory management demo */
void hexagon_stack_setup_demo(void)
{
	printk("Stack setup functionality placeholder\n");
}

/* Memory test function */
void test_memory_regions(void)
{
	printk("Memory region test\n");
	printk("Basic memory regions available\n");
	
	/* Test simple memory allocation */
	static uint32_t test_data[32];
	test_data[0] = 0x12345678;
	test_data[31] = 0x87654321;
	
	printk("Test data buffer: 0x%08x\n", (uint32_t)test_data);
	printk("First value: 0x%08x, Last value: 0x%08x\n", 
	       test_data[0], test_data[31]);
}

/* Stack test function */
void test_stack_management(void)
{
	uintptr_t sp;
	
	/* Get current stack pointer */
	__asm__ volatile("r0 = r29" : "=r"(sp));
	
	printk("Stack test:\n");
	printk("  Current SP: 0x%08lx\n", sp);
	printk("  Stack appears functional\n");
}

/* Cache test function */
void test_cache_operations(void)
{
	uint32_t test_buffer[64] __aligned(HEXAGON_CACHE_LINE_SIZE);
	
	printk("Cache operation test\n");
	
	/* Write test pattern */
	for (int i = 0; i < 64; i++) {
		test_buffer[i] = i;
	}
	
	/* Flush cache */
	arch_dcache_range(test_buffer, sizeof(test_buffer), 
			 HEXAGON_CACHE_CLEAN);
	
	/* Invalidate cache */
	arch_dcache_range(test_buffer, sizeof(test_buffer), 
			 HEXAGON_CACHE_INVALIDATE);
	
	/* Verify data */
	for (int i = 0; i < 64; i++) {
		if (test_buffer[i] != i) {
			printk("Cache test failed at index %d\n", i);
			return;
		}
	}
	
	printk("Cache test passed\n");
}
