/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_MEMORY_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_MEMORY_H_

#include <zephyr/types.h>
#include <zephyr/sys/util.h>

/* Memory region types */
#define HEXAGON_MEM_REGION_CODE    0x01
#define HEXAGON_MEM_REGION_DATA    0x02
#define HEXAGON_MEM_REGION_STACK   0x04
#define HEXAGON_MEM_REGION_DEVICE  0x08
#define HEXAGON_MEM_REGION_CACHED  0x10
#define HEXAGON_MEM_REGION_WRITE   0x20
#define HEXAGON_MEM_REGION_EXEC    0x40

/* Memory alignment requirements */
#define HEXAGON_MMU_PAGE_SIZE      4096
#define HEXAGON_CACHE_LINE_SIZE    32
#define HEXAGON_STACK_ALIGN        8

/* Memory map structure for vmnewmap */
struct hexagon_mem_map {
	uint32_t virt_addr;
	uint32_t phys_addr;
	uint32_t size;
	uint32_t flags;
};

/* Cache operations */
enum hexagon_cache_op {
	HEXAGON_CACHE_CLEAN = 0,
	HEXAGON_CACHE_INVALIDATE = 1,
	HEXAGON_CACHE_CLEAN_INVALIDATE = 2,
	HEXAGON_CACHE_SYNC = 3
};

/* Memory barrier */
static inline void hexagon_mb(void)
{
	__asm__ volatile("barrier" ::: "memory");
}

/* Data memory barrier */
static inline void hexagon_dmb(void)
{
	__asm__ volatile("dccleana(r0)" ::: "memory");
}

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_MEMORY_H_ */
