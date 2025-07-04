/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_CPU_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_CPU_H_

#include <zephyr/types.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/arch/hexagon/arch.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hexagon CPU registers and bits */
#define HEXAGON_USR_IE_BIT      0  /* Interrupt enable bit in USR */
#define HEXAGON_USR_UM_BIT      31 /* User mode bit in USR */

/* Stack alignment requirement */
#define STACK_ALIGN 8
#define ARCH_STACK_PTR_ALIGN 8

/* Cache line size */
#define HEXAGON_DCACHE_LINE_SIZE 32
#define HEXAGON_ICACHE_LINE_SIZE 32

#ifndef _ASMLANGUAGE

/* Read USR register */
static inline uint32_t hexagon_get_usr(void)
{
	uint32_t usr;
	__asm__ __volatile__("usr = %0" : "=r"(usr));
	return usr;
}

/* Write USR register */
static inline void hexagon_set_usr(uint32_t usr)
{
	__asm__ __volatile__("%0 = usr" :: "r"(usr));
}

/* Enable interrupts */
static ALWAYS_INLINE unsigned int arch_irq_unlock(unsigned int key)
{
	hexagon_set_usr(key);
	return key;
}

/* Disable interrupts */
static ALWAYS_INLINE unsigned int arch_irq_lock(void)
{
	uint32_t usr = hexagon_get_usr();
	uint32_t new_usr = usr & ~(1 << HEXAGON_USR_IE_BIT);
	hexagon_set_usr(new_usr);
	return usr;
}

/* Test if interrupts are enabled */
static ALWAYS_INLINE bool arch_irq_unlocked(unsigned int key)
{
	return (key & (1 << HEXAGON_USR_IE_BIT)) != 0;
}

/* Memory barrier */
static inline void arch_mb(void)
{
	__asm__ __volatile__("barrier" ::: "memory");
}

/* No-op */
static ALWAYS_INLINE void arch_nop(void)
{
	__asm__ __volatile__("nop");
}

#endif /* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_CPU_H_ */
