
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <irq.h>

#include <tracing/tracing.h>

static ALWAYS_INLINE void hexagon_idle(unsigned int key)
{
	sys_trace_idle();

	/* unlock interrupts */
	irq_unlock(key);

	/* wait for interrupt */
	__asm__ volatile("wait(r0)");
}

void arch_cpu_idle(void)
{
	hexagon_idle(1);
}

void arch_cpu_atomic_idle(unsigned int key)
{
	hexagon_idle(key);
}
