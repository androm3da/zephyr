/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/arch/common/semihost.h>

long semihost_exec(enum semihost_instr instr, void *args)
{
	register unsigned long r0 __asm__("r0") = instr;
	register void *r1 __asm__("r1") = args;
	register long ret __asm__("r0");

	__asm__ __volatile__(
		"r0 = %1\n\t"
		"r1 = %2\n\t"
		"trap0(#0)\n\t"
		"%0 = r0"
		: "=r"(ret)
		: "r"(r0), "r"(r1)
		: "memory");

	return ret;
}