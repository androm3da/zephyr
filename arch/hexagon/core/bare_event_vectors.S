/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>

GTEXT(__start)
GTEXT(__reset_handler)

/* Reset vector - must be aligned */
SECTION_FUNC(reset, __start)
	jump __reset_handler
	jump ##_fatal_error

/* Main reset handler */
SECTION_FUNC(TEXT, __reset_handler)
	/* Disable interrupts */
	r0 = #0

	/* Set up initial stack pointer */
	r29 = ##_interrupt_stack_end

	/* Clear frame pointer */
	r30 = #0

	/* Set event vector base */
	r0 = ##__start

	/* Initialize BSS section */
	r0 = ##__bss_start
	r1 = ##__bss_end
	r2 = #0

	p0 = cmp.gtu(r1, r0)
	if (!p0) jump #_bss_done

_bss_loop:
	memb(r0++#1) = r2
	p0 = cmp.ltu(r0, r1)
	if (p0) jump #_bss_loop

_bss_done:
	/* Copy initialized data from ROM to RAM if XIP */
#ifdef CONFIG_XIP
	r0 = ##__data_region_load_start
	r1 = ##__data_region_start
	r2 = ##__data_region_end

	p0 = cmp.eq(r0, r1)
	if (p0) jump #_data_done
	p0 = cmp.gtu(r2, r1)
	if (!p0) jump #_data_done

_data_loop:
	r3 = memb(r0++#1)
	memb(r1++#1) = r3
	p0 = cmp.ltu(r1, r2)
	if (p0) jump #_data_loop

_data_done:
#endif

	/* Jump to C initialization */
	call ##z_cstart

	/* Should never reach here */
_hang:
	jump #_hang

_fatal_error:
	/* r0 contains error code */
	jump #_hang
