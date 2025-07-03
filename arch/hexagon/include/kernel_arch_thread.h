/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_THREAD_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

/* Hexagon callee-saved registers */
struct hexagon_callee_saved {
    uint32_t r16;
    uint32_t r17;
    uint32_t r18;
    uint32_t r19;
    uint32_t r20;
    uint32_t r21;
    uint32_t r22;
    uint32_t r23;
    uint32_t r24;
    uint32_t r25;
    uint32_t r26;
    uint32_t r27;
    uint32_t fp;  /* r30 - frame pointer */
    uint32_t sp;  /* r29 - stack pointer */
    uint32_t lr;  /* r31 - link register */
    uint32_t p3_0; /* Predicate registers */
    uint32_t usr;  /* User status register */
};

struct _thread_arch {
    struct hexagon_callee_saved callee_saved;
    uint32_t swap_return_value;
    void *stack_top;
    uint32_t stack_size;
#ifdef CONFIG_THREAD_LOCAL_STORAGE
    uint32_t tls;
#endif
};

typedef struct _thread_arch _thread_arch_t;

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_THREAD_H_ */
