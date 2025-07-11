/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_STACK_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_STACK_H_

/* Stack alignment requirements */
#define ARCH_STACK_PTR_ALIGN 8

/* No special reserved space needed for basic operation */
#define ARCH_THREAD_STACK_RESERVED 0
#define ARCH_KERNEL_STACK_RESERVED 0

/* Stack object alignment */
#define ARCH_THREAD_STACK_OBJ_ALIGN ARCH_STACK_PTR_ALIGN
#define ARCH_KERNEL_STACK_OBJ_ALIGN ARCH_STACK_PTR_ALIGN

/* Stack size alignment */
#define ARCH_THREAD_STACK_SIZE_ALIGN ARCH_STACK_PTR_ALIGN

#ifdef CONFIG_HEXAGON_STACK_PROTECTION
/* Additional space for stack guard regions */
#define ARCH_THREAD_STACK_GUARD_SIZE 32
#define ARCH_KERNEL_STACK_GUARD_SIZE 32
#else
#define ARCH_THREAD_STACK_GUARD_SIZE 0
#define ARCH_KERNEL_STACK_GUARD_SIZE 0
#endif

/* Stack frame overhead for context switching */
#define ARCH_THREAD_STACK_FRAME_OVERHEAD 256

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_STACK_H_ */
