
/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/arch/common/semihost.h>

/*
 * Hexagon semihosting implementation using trap0 instruction
 * Following ARM semihosting conventions but adapted for Hexagon trap mechanism
 */
long semihost_exec(enum semihost_instr instr, void *args)
{
	register long r0 __asm__("r0") = instr;
	register void *r1 __asm__("r1") = args;
	register long ret __asm__("r0");

	__asm__ volatile (
		"trap0(#0)"
		: "=r" (ret) : "r" (r0), "r" (r1) : "memory");

	return ret;
}
