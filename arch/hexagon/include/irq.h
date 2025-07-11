/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_IRQ_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_IRQ_H_

#include <zephyr/irq.h>
#include <zephyr/sw_isr_table.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hexagon supports up to 64 interrupts in the VM model */
#define ARCH_IRQ_COUNT 64

/* Event numbers */
#define HEXAGON_EVENT_MACHINE_CHECK     1
#define HEXAGON_EVENT_GENERAL_EXCEPTION 2
#define HEXAGON_EVENT_TRAP0             5
#define HEXAGON_EVENT_INTERRUPT         7

/* Interrupt priority levels (0-63, 0 is highest) */
#define HEXAGON_IRQ_PRIORITY_HIGHEST 0
#define HEXAGON_IRQ_PRIORITY_LOWEST  63

/* Special IRQ numbers */
#define HEXAGON_NO_PENDING_IRQ 0xFFFFFFFF

#ifndef _ASMLANGUAGE

/* Interrupt lock key type */
typedef uint32_t arch_irq_lock_key_t;

/* Get current interrupt enable state and disable interrupts */
static ALWAYS_INLINE unsigned int arch_irq_lock(void)
{
	uint32_t key;

	__asm__ volatile("trap1(#0x4)\n\t" /* vmgetie - get current state */
			 "r1 = #0\n\t"     /* prepare to disable */
			 "trap1(#0x3)"     /* vmsetie(0) - disable interrupts */
			 : "=r"(key)
			 :
			 : "r0", "r1", "memory");

	return key;
}

/* Restore interrupt state */
static ALWAYS_INLINE void arch_irq_unlock(unsigned int key)
{
	__asm__ volatile("r0 = %0\n\t"
			 "trap1(#0x3)" /* vmsetie - restore state */
			 :
			 : "r"(key & 1)
			 : "r0", "memory");
}

/* Test if interrupts are locked */
static ALWAYS_INLINE bool arch_irq_unlocked(unsigned int key)
{
	return (key & 1) != 0;
}

/* Enable specific IRQ */
void arch_irq_enable(unsigned int irq);

/* Disable specific IRQ */
void arch_irq_disable(unsigned int irq);

/* Check if IRQ is enabled */
int arch_irq_is_enabled(unsigned int irq);

/* Connect IRQ at runtime */
int arch_irq_connect_dynamic(unsigned int irq, unsigned int priority,
			     void (*routine)(const void *parameter), const void *parameter,
			     uint32_t flags);

/* Hexagon-specific interrupt operations */
void hexagon_irq_priority_set(unsigned int irq, unsigned int priority);
int hexagon_irq_trigger(unsigned int irq);

/* Initialize interrupt handling */
void z_hexagon_irq_init(void);

/* Trigger a software interrupt */
int hexagon_irq_trigger(unsigned int irq);

#endif /* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_IRQ_H_ */
