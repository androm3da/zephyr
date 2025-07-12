/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_

#include <zephyr/types.h>

#ifndef _ASMLANGUAGE

/* GDB register definitions for Hexagon */
#define GDB_HEXAGON_R0       0
#define GDB_HEXAGON_R31      31
#define GDB_HEXAGON_PC       32
#define GDB_HEXAGON_USR      33
#define GDB_HEXAGON_GP       34
#define GDB_HEXAGON_UGP      35
#define GDB_HEXAGON_LC0      36
#define GDB_HEXAGON_LC1      37
#define GDB_HEXAGON_SA0      38
#define GDB_HEXAGON_SA1      39
#define GDB_HEXAGON_P0       40
#define GDB_HEXAGON_P3       43
#define GDB_HEXAGON_NUM_REGS 44

/* Hexagon break instruction */
#define HEXAGON_BREAK_INSN 0x6c20c000

/* Maximum number of breakpoints */
#ifndef CONFIG_GDB_MAX_BREAKPOINTS
#define CONFIG_GDB_MAX_BREAKPOINTS 4
#endif

/* GDB context structure - required by Zephyr's gdbstub subsystem */
struct gdb_ctx {
	unsigned int exception;              /* Exception reason */
	uint32_t regs[GDB_HEXAGON_NUM_REGS]; /* Register cache */
	bool stopped;                        /* Execution stopped flag */

	/* Breakpoint support */
	struct {
		uint32_t addr;
		uint32_t saved_insn;
		bool active;
	} breakpoints[CONFIG_GDB_MAX_BREAKPOINTS];
};

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_ */
