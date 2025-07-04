/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/tracing/tracing.h>
#include <zephyr/irq.h>
#include <zephyr/sw_isr_table.h>

extern void z_hexagon_fatal_error(unsigned int reason, const struct arch_esf *esf);

struct _isr_table_entry _sw_isr_table[CONFIG_NUM_IRQS];

/* HVM constants */
#define HVM_TRAP0_VMSETIE 3
#define HVM_TRAP0_VMGETIE 4

/* Simple stubs for VM functions */
static inline void hvm_trap0_2(int op, int arg1, int arg2) { }
static inline int hvm_trap0_1(int op, int arg) { return 0; }

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
int arch_irq_connect_dynamic(unsigned int irq, unsigned int priority,
		      void (*routine)(const void *parameter),
		      const void *parameter, uint32_t flags)
{
	if (irq >= CONFIG_NUM_IRQS) {
		return -EINVAL;
	}

	_sw_isr_table[irq].arg = parameter;
	_sw_isr_table[irq].isr = routine;
	return 0;
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

/* Spurious interrupt handler */
void z_irq_spurious(const void *parameter)
{
	ARG_UNUSED(parameter);
	/* Spurious interrupt - do nothing */
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
