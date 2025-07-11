/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>

/* Simple interrupt controller implementation */
/* In real hardware, this would interface with actual INTC */

/* Interrupt constants */
#define ARCH_IRQ_COUNT              64
#define HEXAGON_IRQ_PRIORITY_LOWEST 63
#define HEXAGON_NO_PENDING_IRQ      0xFFFFFFFF

static uint64_t intc_enabled_mask;
static uint64_t intc_pending_mask;
static uint8_t intc_priorities[ARCH_IRQ_COUNT];

/* Get highest priority pending interrupt */
uint32_t hexagon_intc_get_pending(void)
{
	uint64_t pending = intc_pending_mask & intc_enabled_mask;
	int highest_prio = HEXAGON_IRQ_PRIORITY_LOWEST + 1;
	int highest_irq = HEXAGON_NO_PENDING_IRQ;

	if (!pending) {
		return HEXAGON_NO_PENDING_IRQ;
	}

	/* Find highest priority pending interrupt */
	for (int i = 0; i < ARCH_IRQ_COUNT; i++) {
		if ((pending & BIT64(i)) && (intc_priorities[i] < highest_prio)) {
			highest_prio = intc_priorities[i];
			highest_irq = i;
		}
	}

	return highest_irq;
}

/* Acknowledge interrupt */
void hexagon_intc_ack(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		intc_pending_mask &= ~BIT64(irq);
	}
}

/* Set interrupt pending (for software interrupts) */
void hexagon_intc_set_pending(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		intc_pending_mask |= BIT64(irq);
	}
}

/* Enable interrupt in local controller */
void hexagon_intc_enable(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		intc_enabled_mask |= BIT64(irq);
	}
}

/* Disable interrupt in local controller */
void hexagon_intc_disable(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		intc_enabled_mask &= ~BIT64(irq);
	}
}

/* Set interrupt priority */
void hexagon_intc_set_priority(uint32_t irq, uint32_t priority)
{
	if (irq < ARCH_IRQ_COUNT && priority <= HEXAGON_IRQ_PRIORITY_LOWEST) {
		intc_priorities[irq] = priority;
	}
}

/* Initialize interrupt controller */
void hexagon_intc_init(void)
{
	/* Clear all state */
	intc_enabled_mask = 0;
	intc_pending_mask = 0;

	/* Set default priorities */
	for (int i = 0; i < ARCH_IRQ_COUNT; i++) {
		intc_priorities[i] = HEXAGON_IRQ_PRIORITY_LOWEST;
	}
}
