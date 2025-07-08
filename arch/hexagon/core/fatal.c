/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>

void arch_system_halt(unsigned int reason)
{
	/* Use Hexagon stop instruction to halt the system */
	__asm__ volatile("stop(r0)" ::: "r0", "memory");

	/* Should never reach here, but include infinite loop as fallback */
	while (1) {
		arch_cpu_idle();
	}
}
