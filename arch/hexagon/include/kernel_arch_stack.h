/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_STACK_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_STACK_H_

#ifndef _ASMLANGUAGE

#include <zephyr/types.h>

/* Stack object requirements */
#define ARCH_STACK_PTR_ALIGN            8
#define ARCH_THREAD_STACK_OBJ_ALIGN(size)     8
#define ARCH_KERNEL_STACK_OBJ_ALIGN     8
#define ARCH_THREAD_STACK_SIZE_ALIGN(size)    \
	ROUND_UP(size, ARCH_STACK_PTR_ALIGN)

/* Reserved space in stack objects */
#define ARCH_THREAD_STACK_RESERVED      0
#define ARCH_KERNEL_STACK_RESERVED      0

#ifdef CONFIG_MPU_STACK_GUARD
/* Guard region size for MPU-based protection */
#define ARCH_KERNEL_STACK_GUARD_SIZE    32
#define ARCH_THREAD_STACK_GUARD_SIZE    32
#else
#define ARCH_KERNEL_STACK_GUARD_SIZE    0
#define ARCH_THREAD_STACK_GUARD_SIZE    0
#endif

/* Stack initialization value for debugging */
#define ARCH_STACK_INIT_VALUE 0xAAAAAAAA

/* Get stack bounds for current thread */
static inline void z_hexagon_get_stack_bounds(uintptr_t *low, uintptr_t *high)
{
	struct k_thread *thread = k_current_get();
	
	*low = thread->stack_info.start;
	*high = thread->stack_info.start + thread->stack_info.size;
}

/* Check if address is within current stack */
static inline bool z_hexagon_is_in_stack(uintptr_t addr)
{
	uintptr_t low, high;
	z_hexagon_get_stack_bounds(&low, &high);
	return (addr >= low) && (addr < high);
}

#endif /* _ASMLANGUAGE */

/* Stack canary for overflow detection */
#ifdef CONFIG_STACK_CANARIES
#define ARCH_STACK_CANARY_SIZE sizeof(uint32_t)
#define ARCH_STACK_CANARY_VALUE 0xDEADBEEF
#else
#define ARCH_STACK_CANARY_SIZE 0
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_STACK_H_ */
