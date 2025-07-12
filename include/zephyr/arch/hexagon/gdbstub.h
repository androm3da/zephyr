/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_

#include <zephyr/types.h>

#ifndef _ASMLANGUAGE

/* GDB register definitions for Hexagon */
#define GDB_HEXAGON_R0       0
#define GDB_HEXAGON_R1       1
#define GDB_HEXAGON_R2       2
#define GDB_HEXAGON_R3       3
#define GDB_HEXAGON_R4       4
#define GDB_HEXAGON_R5       5
#define GDB_HEXAGON_R6       6
#define GDB_HEXAGON_R7       7
#define GDB_HEXAGON_R8       8
#define GDB_HEXAGON_R9       9
#define GDB_HEXAGON_R10      10
#define GDB_HEXAGON_R11      11
#define GDB_HEXAGON_R12      12
#define GDB_HEXAGON_R13      13
#define GDB_HEXAGON_R14      14
#define GDB_HEXAGON_R15      15
#define GDB_HEXAGON_R16      16
#define GDB_HEXAGON_R17      17
#define GDB_HEXAGON_R18      18
#define GDB_HEXAGON_R19      19
#define GDB_HEXAGON_R20      20
#define GDB_HEXAGON_R21      21
#define GDB_HEXAGON_R22      22
#define GDB_HEXAGON_R23      23
#define GDB_HEXAGON_R24      24
#define GDB_HEXAGON_R25      25
#define GDB_HEXAGON_R26      26
#define GDB_HEXAGON_R27      27
#define GDB_HEXAGON_R28      28
#define GDB_HEXAGON_R29      29
#define GDB_HEXAGON_R30      30
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
#define GDB_HEXAGON_P1       41
#define GDB_HEXAGON_P2       42
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

/* Function declarations */
bool arch_gdb_memory_readable(uintptr_t addr, size_t size);
bool arch_gdb_memory_writable(uintptr_t addr, size_t size);
int arch_gdb_add_breakpoint(struct gdb_ctx *ctx, uint8_t type, uintptr_t addr, uint32_t kind);
int arch_gdb_remove_breakpoint(struct gdb_ctx *ctx, uint8_t type, uintptr_t addr, uint32_t kind);

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_ */
