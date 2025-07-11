/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <hexagon_vm.h>

/* Hexagon VM-based interrupt controller implementation */

/* Interrupt constants */
#define ARCH_IRQ_COUNT         64
#define HEXAGON_NO_PENDING_IRQ 0xFFFFFFFF

/* Get highest priority pending interrupt */
uint32_t hexagon_intc_get_pending(void)
{
	/* Use VM hypercall to get pending interrupt */
	int32_t irq = hexagon_vm_intop_get();

	if (irq < 0 || irq >= ARCH_IRQ_COUNT) {
		return HEXAGON_NO_PENDING_IRQ;
	}

	return (uint32_t)irq;
}

/* Acknowledge interrupt using VM hypercalls */
void hexagon_intc_ack(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		/* Use VM hypercall for end-of-interrupt (global enable) */
		hexagon_vm_intop_globen((int32_t)irq);
	}
}

/* Set interrupt pending (for software interrupts) */
void hexagon_intc_set_pending(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		/* Use VM hypercall to post interrupt */
		hexagon_vm_intop_post((int32_t)irq, 0);
	}
}

/* Enable interrupt in local controller using VM hypercalls */
void hexagon_intc_enable(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		/* Use VM hypercall for local enable */
		hexagon_vm_intop_locen((int32_t)irq);
	}
}

/* Disable interrupt in local controller using VM hypercalls */
void hexagon_intc_disable(uint32_t irq)
{
	if (irq < ARCH_IRQ_COUNT) {
		/* Use VM hypercall for local disable */
		hexagon_vm_intop_locdis((int32_t)irq);
	}
}

/* Set interrupt priority - not supported by VM operations, stub only */
void hexagon_intc_set_priority(uint32_t irq, uint32_t priority)
{
	/* VM interrupt operations don't support priority setting */
	/* This is a stub for compatibility */
	ARG_UNUSED(irq);
	ARG_UNUSED(priority);
}

/* Trigger software interrupt */
int hexagon_irq_trigger(unsigned int irq)
{
	if (irq >= ARCH_IRQ_COUNT) {
		return -1;
	}

	/* Use VM hypercall to post interrupt */
	hexagon_intc_set_pending(irq);
	return 0;
}

/* Initialize interrupt controller using VM hypercalls */
void hexagon_intc_init(void)
{
	/* Disable all interrupts initially */
	for (int i = 0; i < ARCH_IRQ_COUNT; i++) {
		hexagon_vm_intop_locdis(i);
	}
}
