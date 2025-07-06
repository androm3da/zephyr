/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_ARCH_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_ARCH_H_

/* Stack pointer alignment requirement */
#define ARCH_STACK_PTR_ALIGN 8

/* Thread pointer alignment - same as stack */
#define ARCH_THREAD_STACK_OBJ_ALIGN ARCH_STACK_PTR_ALIGN

/* Stack alignment - same as stack pointer alignment */
#define STACK_ALIGN ARCH_STACK_PTR_ALIGN

/* Exception stack frame size */
#define ARCH_ESFT_SIZEOF 24

/* Include architecture-specific structures */
#include <zephyr/arch/hexagon/structs.h>

#ifndef _ASMLANGUAGE
#include <zephyr/arch/hexagon/thread.h>

/* Exception stack frame for Hexagon */
struct arch_esf {
    uint32_t usr;    /* User status register */
    uint32_t elr;    /* Exception return address */
    uint32_t sp;     /* Stack pointer */
    uint32_t r0;     /* General registers */
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
};

/* Interrupt management functions */
static inline unsigned int arch_irq_lock(void)
{
    /* For now, return 0 indicating interrupts were enabled */
    return 0;
}

static inline void arch_irq_unlock(unsigned int key)
{
    /* No-op for now */
    (void)key;
}

static inline bool arch_irq_unlocked(unsigned int key)
{
    return key == 0;
}

/* Cycle counter functions */
static inline uint32_t arch_k_cycle_get_32(void)
{
    uint32_t val;
    __asm__ __volatile__("%0 = pcyclelo\n" : "=r"(val));
    return val;
}

static inline uint64_t arch_k_cycle_get_64(void)
{
    uint64_t val;
    __asm__ __volatile__("%0 = pcycle\n" : "=r"(val));
    return val;
}

/* Timer function */
static inline uint32_t z_timer_cycle_get_32(void)
{
    return arch_k_cycle_get_32();
}

/* No-op function */
static inline void arch_nop(void)
{
    /* No operation */
}

/* Find first set bit functions */
static inline unsigned int find_lsb_set(uint32_t op)
{
    if (op == 0) {
        return 0;
    }
    return __builtin_ctz(op) + 1;
}

static inline unsigned int find_msb_set(uint32_t op)
{
    if (op == 0) {
        return 0;
    }
    return 32 - __builtin_clz(op);
}

/* ISR detection */
static inline bool arch_is_in_isr(void)
{
    /* For now, return false indicating we're not in an ISR */
    return false;
}

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_ARCH_H_ */