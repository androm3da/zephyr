/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * Macros for Hexagon register manipulations
 * inspired by linux/arch/mips/include/asm/mipsregs.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_HEXAGONREGS_H_
#define _ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_HEXAGONREGS_H_

#define SYS_SSR ssr

/* SSR CAUSE bits */
#define SSR_CAUSE_MASK 0x0000007f

/* SSR User Mode, EXception, Guest Mode bits: */
#define SSR_UM BIT(16)
#define SSR_EX BIT(17)
#define SSR_GM BIT(19)

#define _hexagon_read_sys_reg_(reg, reg_t)                                \
	({                                                                    \
		reg_t val;                                                        \
		__asm__ __volatile__("%0 = " STRINGIFY(reg) "\n" : "=r"(val));    \
		val;                                                              \
	})
#define _hexagon_read_sys_reg(reg) _hexagon_read_sys_reg_(reg, uint32_t)
#define _hexagon_read_sys_reg_pair(reg) _hexagon_read_sys_reg_(reg, uint64_t)

#define read_ssr() _hexagon_read_sys_reg(ssr)
#define read_badva0() _hexagon_read_sys_reg(badva0)
#define read_badva1() _hexagon_read_sys_reg(badva1)

#endif /* _ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_HEXAGONREGS_H_ */
