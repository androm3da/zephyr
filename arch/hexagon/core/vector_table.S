/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>

GTEXT(_vector_table)
GTEXT(_isr_wrapper)

/* Vector table - must be 1KB aligned */
SECTION_FUNC(TEXT, _vector_table)
.align 1024

/* Reset vector */
.org 0x0
	jump _reset_handler

/* General exception */
.org 0x100
	jump _K_enter_genex

/* Interrupt vectors */
.org 0x200
.rept 32
	jump _K_enter_interrupt
	nop
	nop
	nop
.endr

/* ISR wrapper to save/restore context */
SECTION_FUNC(TEXT, _isr_wrapper)
	/* Save caller-saved registers - simplified stack manipulation */
	r29 = add(r29, #-56)  /* allocframe replacement */
	memw(r29+#0) = r0
	memw(r29+#4) = r1
	memw(r29+#8) = r2
	memw(r29+#12) = r3
	memw(r29+#16) = r4
	memw(r29+#20) = r5
	memw(r29+#24) = r6
	memw(r29+#28) = r7
	memw(r29+#32) = r8
	memw(r29+#36) = r9
	memw(r29+#40) = r10
	memw(r29+#44) = r11
	memw(r29+#48) = r12
	memw(r29+#52) = r13

	/* Get IRQ number - simplified without extractu */
	r0 = usr
	r1 = #0x1F
	r0 = and(r0, r1)

	/* Call C handler */
	call z_hexagon_irq_handler

	/* Restore registers */
	r0 = memw(r29+#0)
	r1 = memw(r29+#4)
	r2 = memw(r29+#8)
	r3 = memw(r29+#12)
	r4 = memw(r29+#16)
	r5 = memw(r29+#20)
	r6 = memw(r29+#24)
	r7 = memw(r29+#28)
	r8 = memw(r29+#32)
	r9 = memw(r29+#36)
	r10 = memw(r29+#40)
	r11 = memw(r29+#44)
	r12 = memw(r29+#48)
	r13 = memw(r29+#52)
	r29 = add(r29, #56)  /* deallocframe replacement */

	/* Return from interrupt - use jumpr instead of rte */
	jumpr r31

SECTION_FUNC(TEXT, _general_exception)
	/* Handle general exceptions */
	r29 = add(r29, #-8)  /* allocframe replacement */
	r0 = #1  /* K_ERR_CPU_EXCEPTION equivalent */
	r1 = r29
	call z_hexagon_fatal_error
	/* Should not return */
	r29 = add(r29, #8)  /* deallocframe replacement */
	jumpr r31
