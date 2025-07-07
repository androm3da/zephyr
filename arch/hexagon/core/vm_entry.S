/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hexagon Virtual Machine event entry/exit for Zephyr
 */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
/* Include HVM constants directly since we can't include C header */

/* HVM event cause definitions */
#define HVM_VMEST_CAUSE_TRAP       0x01
#define HVM_VMEST_CAUSE_TRAP0      0x02
#define HVM_VMEST_CAUSE_MACHCHECK  0x03
#define HVM_VMEST_CAUSE_GENEX      0x04
#define HVM_VMEST_CAUSE_PMUREAD    0x05
#define HVM_VMEST_CAUSE_INTERRUPT  0x06

/* HVM trap1 hypercall numbers */
#define HVM_TRAP1_VMRESUME         1

/* Simplified pt_regs structure offsets for Zephyr */
#define _PT_REGS_SIZE 168

/*
 * Macro to save processor state for HVM events
 * Simplified version for Zephyr - save essential registers only
 */
.macro save_hvm_regs
	r29 = add(r29, #-_PT_REGS_SIZE)  /* allocframe replacement */

	/* Save general purpose registers */
	memw(r29 + #0) = r0
	memw(r29 + #4) = r1
	memw(r29 + #8) = r2
	memw(r29 + #12) = r3
	memw(r29 + #16) = r4
	memw(r29 + #20) = r5
	memw(r29 + #24) = r6
	memw(r29 + #28) = r7
	memw(r29 + #32) = r8
	memw(r29 + #36) = r9
	memw(r29 + #40) = r10
	memw(r29 + #44) = r11
	memw(r29 + #48) = r12
	memw(r29 + #52) = r13
	memw(r29 + #56) = r14
	memw(r29 + #60) = r15
	memw(r29 + #64) = r16
	memw(r29 + #68) = r17
	memw(r29 + #72) = r18
	memw(r29 + #76) = r19
	memw(r29 + #80) = r20
	memw(r29 + #84) = r21
	memw(r29 + #88) = r22
	memw(r29 + #92) = r23
	memw(r29 + #96) = r24
	memw(r29 + #100) = r25
	memw(r29 + #104) = r26
	memw(r29 + #108) = r27
	memw(r29 + #112) = r28

	/* Save essential system registers */
	r16 = usr
	memw(r29 + #116) = r16
	r16 = gp
	memw(r29 + #120) = r16
	r16 = ugp
	memw(r29 + #124) = r16
.endm

/*
 * Macro to restore processor state for HVM events
 */
.macro restore_hvm_regs
	/* Restore essential system registers */
	r16 = memw(r29 + #116)
	usr = r16
	r16 = memw(r29 + #120)
	gp = r16
	r16 = memw(r29 + #124)
	ugp = r16

	/* Restore general purpose registers */
	r0 = memw(r29 + #0)
	r1 = memw(r29 + #4)
	r2 = memw(r29 + #8)
	r3 = memw(r29 + #12)
	r4 = memw(r29 + #16)
	r5 = memw(r29 + #20)
	r6 = memw(r29 + #24)
	r7 = memw(r29 + #28)
	r8 = memw(r29 + #32)
	r9 = memw(r29 + #36)
	r10 = memw(r29 + #40)
	r11 = memw(r29 + #44)
	r12 = memw(r29 + #48)
	r13 = memw(r29 + #52)
	r14 = memw(r29 + #56)
	r15 = memw(r29 + #60)
	r16 = memw(r29 + #64)
	r17 = memw(r29 + #68)
	r18 = memw(r29 + #72)
	r19 = memw(r29 + #76)
	r20 = memw(r29 + #80)
	r21 = memw(r29 + #84)
	r22 = memw(r29 + #88)
	r23 = memw(r29 + #92)
	r24 = memw(r29 + #96)
	r25 = memw(r29 + #100)
	r26 = memw(r29 + #104)
	r27 = memw(r29 + #108)
	r28 = memw(r29 + #112)

	r29 = add(r29, #_PT_REGS_SIZE)  /* deallocframe replacement */
.endm

/*
 * HVM trap0 handler - for system calls and semihosting
 */
GTEXT(hvm_do_trap0)
SECTION_FUNC(TEXT, hvm_do_trap0)
	save_hvm_regs

	/* Set up argument: struct pt_regs *regs */
	r0 = r29
	call z_hexagon_do_trap0

	restore_hvm_regs
	trap1(#HVM_TRAP1_VMRESUME)

/*
 * HVM machine check handler
 */
GTEXT(hvm_do_machcheck)
SECTION_FUNC(TEXT, hvm_do_machcheck)
	save_hvm_regs

	/* Set up argument: struct pt_regs *regs */
	r0 = r29
	call z_hexagon_do_machcheck

	restore_hvm_regs
	trap1(#HVM_TRAP1_VMRESUME)

/*
 * HVM general exception handler
 */
GTEXT(hvm_do_genex)
SECTION_FUNC(TEXT, hvm_do_genex)
	save_hvm_regs

	/* Set up argument: struct pt_regs *regs */
	r0 = r29
	call z_hexagon_do_genex

	restore_hvm_regs
	trap1(#HVM_TRAP1_VMRESUME)

/*
 * HVM interrupt handler
 */
GTEXT(hvm_do_interrupt)
SECTION_FUNC(TEXT, hvm_do_interrupt)
	save_hvm_regs

	/* Set up argument: struct pt_regs *regs */
	r0 = r29
	call z_hexagon_do_interrupt

	restore_hvm_regs
	trap1(#HVM_TRAP1_VMRESUME)

/*
 * HVM PMU read handler
 */
GTEXT(hvm_do_pmu_read)
SECTION_FUNC(TEXT, hvm_do_pmu_read)
	save_hvm_regs

	/* Set up argument: struct pt_regs *regs */
	r0 = r29
	call z_hexagon_do_pmu_read

	restore_hvm_regs
	trap1(#HVM_TRAP1_VMRESUME)

/*
 * Main HVM event dispatcher
 * This function routes HVM events to appropriate handlers
 */
GTEXT(hvm_event_entry)
SECTION_FUNC(TEXT, hvm_event_entry)
	/* r4 contains the event cause */
	r6 = #HVM_VMEST_CAUSE_TRAP
	p0 = cmp.eq(r4, r6)
	if (p0) jump hvm_do_trap0

	r6 = #HVM_VMEST_CAUSE_TRAP0
	p0 = cmp.eq(r4, r6)
	if (p0) jump hvm_do_trap0

	r6 = #HVM_VMEST_CAUSE_MACHCHECK
	p0 = cmp.eq(r4, r6)
	if (p0) jump hvm_do_machcheck

	r6 = #HVM_VMEST_CAUSE_GENEX
	p0 = cmp.eq(r4, r6)
	if (p0) jump hvm_do_genex

	r6 = #HVM_VMEST_CAUSE_PMUREAD
	p0 = cmp.eq(r4, r6)
	if (p0) jump hvm_do_pmu_read

	r6 = #HVM_VMEST_CAUSE_INTERRUPT
	p0 = cmp.eq(r4, r6)
	if (p0) jump hvm_do_interrupt

	/* Unknown event - just resume */
	trap1(#HVM_TRAP1_VMRESUME)

/*
 * Entry points for different HVM event types
 * These set up the event cause and jump to the main dispatcher
 */
GTEXT(_K_enter_genex)
SECTION_FUNC(TEXT, _K_enter_genex)
	r4 = #HVM_VMEST_CAUSE_GENEX
	jump hvm_event_entry

GTEXT(_K_enter_interrupt)
SECTION_FUNC(TEXT, _K_enter_interrupt)
	r4 = #HVM_VMEST_CAUSE_INTERRUPT
	jump hvm_event_entry

GTEXT(_K_enter_machcheck)
SECTION_FUNC(TEXT, _K_enter_machcheck)
	r4 = #HVM_VMEST_CAUSE_MACHCHECK
	jump hvm_event_entry

GTEXT(_K_enter_pmu_read)
SECTION_FUNC(TEXT, _K_enter_pmu_read)
	r4 = #HVM_VMEST_CAUSE_PMUREAD
	jump hvm_event_entry

GTEXT(_K_enter_trap0)
SECTION_FUNC(TEXT, _K_enter_trap0)
	r4 = #HVM_VMEST_CAUSE_TRAP0
	jump hvm_event_entry

GTEXT(_K_enter_trap)
SECTION_FUNC(TEXT, _K_enter_trap)
	r4 = #HVM_VMEST_CAUSE_TRAP
	jump hvm_event_entry
