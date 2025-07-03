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
.align 10

/* Reset vector */
.org 0x0
	jump _reset_handler

/* General exception */
.org 0x100
	jump _general_exception

/* Interrupt vectors */
.org 0x200
.rept 32
	jump _isr_wrapper
	nop
	nop
	nop
.endr

/* ISR wrapper to save/restore context */
SECTION_FUNC(TEXT, _isr_wrapper)
	/* Save caller-saved registers */
	allocframe(#56)
	memd(sp+#0) = r1:0
	memd(sp+#8) = r3:2
	memd(sp+#16) = r5:4
	memd(sp+#24) = r7:6
	memd(sp+#32) = r9:8
	memd(sp+#40) = r11:10
	memd(sp+#48) = r13:12

	/* Get IRQ number from IPEND register */
	r0 = usr
	r0 = extractu(r0, #5, #0)

	/* Call C handler */
	call z_hexagon_irq_handler

	/* Restore registers */
	r1:0 = memd(sp+#0)
	r3:2 = memd(sp+#8)
	r5:4 = memd(sp+#16)
	r7:6 = memd(sp+#24)
	r9:8 = memd(sp+#32)
	r11:10 = memd(sp+#40)
	r13:12 = memd(sp+#48)
	deallocframe

	/* Return from interrupt */
	rte

SECTION_FUNC(TEXT, _general_exception)
	/* Handle general exceptions */
	allocframe(#8)
	r0 = #K_ERR_CPU_EXCEPTION
	r1 = sp
	call z_hexagon_fatal_error
	/* Should not return */
	deallocframe
	jumpr r31
