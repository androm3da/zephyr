/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/irq.h>
#include <hexagon_vm.h>

static inline uint32_t hexagon_vm_wait_irq(void)
{
	return hexagon_vm_wait(); /* Returns interrupt number that woke us */
}

/* Simple idle - just wait with interrupts enabled */
void arch_cpu_idle(void)
{
	sys_trace_idle();

	/* Ensure interrupts are enabled */
	arch_irq_unlock(0);

	/* Use VM wait */
	hexagon_vm_wait_irq();
}

/* Atomic idle - atomically enable interrupts and wait */
void arch_cpu_atomic_idle(unsigned int key)
{
	sys_trace_idle();

	/* VM wait will atomically enable interrupts and wait */
	/* This prevents race condition between check and wait */

	/* Restore interrupt state and wait atomically */
	if (!arch_irq_unlocked(key)) {
		/* Interrupts were locked, don't idle */
		return;
	}

	/* Safe to idle with interrupts */
	hexagon_vm_wait_irq();
}

/* Power management states */
#ifdef CONFIG_PM
void arch_cpu_sleep(void)
{
	/* Enter deeper sleep state if supported */
	hexagon_vm_wait_irq();
}

void arch_cpu_deep_sleep(void)
{
	/* Enter deepest sleep state if supported */
	/* May require saving additional context */
	hexagon_vm_wait_irq();
}
#endif /* CONFIG_PM */
