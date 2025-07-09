/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/arch/hexagon/vm_ops.h>

GTEXT(__start)
GTEXT(__reset_handler)

/* Reset vector - must be aligned */
SECTION_FUNC(reset, __start)
	jump __reset_handler
	nop
	nop
	nop

/* Main reset handler */
SECTION_FUNC(TEXT, __reset_handler)
	/* Disable interrupts */
	r0 = #0
	trap0(#HEXAGON_VM_vmsetie)

	/* Get VM version for compatibility check */
	r0 = #CONFIG_HEXAGON_VM_VERSION
	trap0(#HEXAGON_VM_vmversion)
	p0 = cmp.eq(r0, #CONFIG_HEXAGON_VM_VERSION)
	if (!p0) jump #_vm_version_error

	/* Set up initial stack pointer */
	r29 = ##(_interrupt_stack + CONFIG_ISR_STACK_SIZE)

	/* Clear frame pointer */
	r30 = #0

	/* Set event vector base */
	r0 = ##_event_vector_table
	trap0(#HEXAGON_VM_vmsetvec)
	p0 = cmp.eq(r0, #0)
	if (!p0) jump #_vmsetvec_error

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

_vm_version_error:
	r0 = #1
	jump #_fatal_error

_vmsetvec_error:
	r0 = #2
	jump #_fatal_error

_fatal_error:
	/* r0 contains error code */
	jump #_hang
