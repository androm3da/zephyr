/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>

void arch_cpu_idle(void)
{
	sys_trace_idle();

	/* Use Hexagon pause instruction with maximum delay */
	__asm__ volatile("pause(#255)" ::: "memory");
}

void arch_cpu_atomic_idle(unsigned int key)
{
	sys_trace_idle();
	irq_unlock(key);

	/* Use Hexagon pause instruction with maximum delay */
	__asm__ volatile("pause(#255)" ::: "memory");
}
