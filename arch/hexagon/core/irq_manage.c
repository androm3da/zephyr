/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/tracing/tracing.h>
#include <zephyr/irq.h>
#include <hexagon_vm.h>

extern void z_hexagon_fatal_error(unsigned int reason, const struct arch_esf *esf);

static struct _isr_table_entry _sw_isr_table[CONFIG_NUM_IRQS];

void arch_irq_enable(unsigned int irq)
{
	if (irq >= CONFIG_NUM_IRQS) {
		return;
	}

	/* Enable specific interrupt through VM */
	hvm_trap0_2(HVM_TRAP0_VMSETIE, irq, 1);
}

void arch_irq_disable(unsigned int irq)
{
	if (irq >= CONFIG_NUM_IRQS) {
		return;
	}

	/* Disable specific interrupt through VM */
	hvm_trap0_2(HVM_TRAP0_VMSETIE, irq, 0);
}

int arch_irq_is_enabled(unsigned int irq)
{
	if (irq >= CONFIG_NUM_IRQS) {
		return 0;
	}

	return hvm_trap0_1(HVM_TRAP0_VMGETIE, irq);
}

/**
 * @brief Connect an ISR to an interrupt
 */
void arch_irq_connect_dynamic(unsigned int irq, unsigned int priority,
		      void (*routine)(const void *parameter),
		      const void *parameter, uint32_t flags)
{
	if (irq >= CONFIG_NUM_IRQS) {
		return;
	}

	_sw_isr_table[irq].arg = parameter;
	_sw_isr_table[irq].isr = routine;
}

/**
 * @brief Common interrupt handler
 */
void z_hexagon_irq_handler(uint32_t irq)
{
	struct _isr_table_entry *entry;

	if (irq >= CONFIG_NUM_IRQS) {
		z_hexagon_fatal_error(K_ERR_SPURIOUS_IRQ, NULL);
		return;
	}

	entry = &_sw_isr_table[irq];

	if (entry->isr) {
		entry->isr(entry->arg);
	}
}

void z_hexagon_irq_init(void)
{
	int i;

	/* Initialize software ISR table */
	for (i = 0; i < CONFIG_NUM_IRQS; i++) {
		_sw_isr_table[i].arg = NULL;
		_sw_isr_table[i].isr = z_irq_spurious;
	}
}
