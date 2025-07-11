/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_FFS_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_FFS_H_

#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Find first set bit (1-based, 0 if no bit set) */
static ALWAYS_INLINE unsigned int arch_ffs(unsigned int value)
{
	unsigned int result;

	if (value == 0) {
		return 0;
	}

	/* Hexagon has ct0 instruction - count trailing zeros */
	__asm__ volatile("%0 = ct0(%1)\n\t"
			 "%0 = add(%0, #1)"
			 : "=r"(result)
			 : "r"(value));

	return result;
}

/* Find last set bit (1-based, 0 if no bit set) */
static ALWAYS_INLINE unsigned int arch_fls(unsigned int value)
{
	unsigned int result;

	if (value == 0) {
		return 0;
	}

	/* Use clb (count leading bits) instruction */
	__asm__ volatile("%0 = clb(%1)\n\t"  /* Count leading zeros */
			 "%0 = sub(#32, %0)" /* Convert to position */
			 : "=r"(result)
			 : "r"(value));

	return result;
}

/* Count trailing zeros */
static ALWAYS_INLINE unsigned int arch_ctz(unsigned int value)
{
	unsigned int result;

	__asm__ volatile("%0 = ct0(%1)" : "=r"(result) : "r"(value));

	return result;
}

/* Count leading zeros */
static ALWAYS_INLINE unsigned int arch_clz(unsigned int value)
{
	unsigned int result;

	__asm__ volatile("%0 = clb(%1)" : "=r"(result) : "r"(value));

	return result;
}

/* Population count (number of set bits) */
static ALWAYS_INLINE unsigned int arch_popcount(unsigned int value)
{
	unsigned int result;

	__asm__ volatile("%0 = popcount(%1)" : "=r"(result) : "r"(value));

	return result;
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_FFS_H_ */
