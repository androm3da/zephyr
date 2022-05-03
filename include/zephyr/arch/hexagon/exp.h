/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * based on include/arch/mips/exp.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_EXP_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_EXP_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>
#include <toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __esf {
	uint32_t r0; /* Caller-saved temporary register */
	uint32_t r1; /* Caller-saved temporary register */
	uint32_t r2; /* Caller-saved temporary register */
	uint32_t r3; /* Caller-saved temporary register */
	uint32_t r4; /* Caller-saved temporary register */
	uint32_t r5; /* Caller-saved temporary register */
	uint32_t r6; /* Caller-saved temporary register */
	uint32_t r7; /* Caller-saved temporary register */
	uint32_t r8; /* Caller-saved temporary register */
	uint32_t r9; /* Caller-saved temporary register */
	uint32_t badva0;
	uint32_t badva1;
	uint32_t elr;
	uint32_t ssr;
};

typedef struct __esf z_arch_esf_t;

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_EXP_H_ */
