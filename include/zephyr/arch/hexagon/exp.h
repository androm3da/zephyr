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

enum hexagon_exception_cause {
    HEXAGON_EXCEPTION_CAUSE_RESET = 0,
    HEXAGON_EXCEPTION_CAUSE_BIU_ERROR = 0x1,
    HEXAGON_EXCEPTION_CAUSE_DOUBLE_EXC = 0x3,
    HEXAGON_EXCEPTION_CAUSE_FETCH_NO_EXEC    = 0x11,
    HEXAGON_EXCEPTION_CAUSE_FETCH_NO_UM      = 0x12,
    HEXAGON_EXCEPTION_CAUSE_INVALID_PACKET      = 0x15,
    HEXAGON_EXCEPTION_CAUSE_ILLEGAL_COPROC      = 0x16,
    HEXAGON_EXCEPTION_CAUSE_INST_CACHE_ERR      = 0x17,
    HEXAGON_EXCEPTION_CAUSE_GUEST_INST_UM       = 0x1a,
    HEXAGON_EXCEPTION_CAUSE_SUP_INST_GM_UM      = 0x1b,
    HEXAGON_EXCEPTION_CAUSE_MULT_WRITES         = 0x1d,
    HEXAGON_EXCEPTION_CAUSE_PC_ALIGN            = 0x1e,
    HEXAGON_EXCEPTION_CAUSE_UNALIGNED_LOAD      = 0x20,
    HEXAGON_EXCEPTION_CAUSE_UNALIGNED_STORE     = 0x21,
};

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_EXP_H_ */
