/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Hexagon architecture GDB stub definitions
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_

/*
 * Hexagon GDB breakpoint instruction.
 *
 * trap0(#0xdb) is used as the software breakpoint instruction.  The encoding
 * 0x5400db0c places the immediate 0xdb in bits[15:8] of the instruction word.
 * The trap0 handler checks this immediate to distinguish GDB breakpoints from
 * syscall traps (trap0(#1)) and other uses.
 */
#define HEXAGON_BREAK_INSN 0x5400db0c

/*
 * Callee-saved register buffer offsets (in bytes).
 * Used by both assembly (event_handlers.S) and C (gdbstub.c) to index
 * into the _gdb_callee_regs[] array.
 *
 * Layout (13 GPRs + 7 special = 20 words = 80 bytes):
 *   [0]  r16    [1]  r17    [2]  r18    [3]  r19
 *   [4]  r20    [5]  r21    [6]  r22    [7]  r23
 *   [8]  r24    [9]  r25    [10] r26    [11] r27
 *   [12] r28
 *   [13] USR    [14] GP     [15] UGP
 *   [16] LC0    [17] LC1    [18] SA0    [19] SA1
 */
#define GDB_CALLEE_R16_OFF   (0 * 4)
#define GDB_CALLEE_R17_OFF   (1 * 4)
#define GDB_CALLEE_R18_OFF   (2 * 4)
#define GDB_CALLEE_R19_OFF   (3 * 4)
#define GDB_CALLEE_R20_OFF   (4 * 4)
#define GDB_CALLEE_R21_OFF   (5 * 4)
#define GDB_CALLEE_R22_OFF   (6 * 4)
#define GDB_CALLEE_R23_OFF   (7 * 4)
#define GDB_CALLEE_R24_OFF   (8 * 4)
#define GDB_CALLEE_R25_OFF   (9 * 4)
#define GDB_CALLEE_R26_OFF   (10 * 4)
#define GDB_CALLEE_R27_OFF   (11 * 4)
#define GDB_CALLEE_R28_OFF   (12 * 4)
#define GDB_CALLEE_USR_OFF   (13 * 4)
#define GDB_CALLEE_GP_OFF    (14 * 4)
#define GDB_CALLEE_UGP_OFF   (15 * 4)
#define GDB_CALLEE_LC0_OFF   (16 * 4)
#define GDB_CALLEE_LC1_OFF   (17 * 4)
#define GDB_CALLEE_SA0_OFF   (18 * 4)
#define GDB_CALLEE_SA1_OFF   (19 * 4)
#define GDB_CALLEE_BUF_SIZE  20

#ifndef _ASMLANGUAGE

#include <zephyr/types.h>
#include <stdbool.h>

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

/* Maximum number of breakpoints */
#ifndef GDB_MAX_BREAKPOINTS
#define GDB_MAX_BREAKPOINTS 4
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
	} breakpoints[GDB_MAX_BREAKPOINTS];
};

/* Callee-saved register buffer shared between assembly and C */
extern uint32_t _gdb_callee_regs[GDB_CALLEE_BUF_SIZE];

/* Entry point from assembly debug handler into gdbstub */
struct event_context;
void z_hexagon_gdb_entry(struct event_context *ctx);

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_GDBSTUB_H_ */
