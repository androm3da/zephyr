/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/arch/hexagon/vm_ops.h>

GTEXT(__start)
GTEXT(__reset_handler)
GTEXT(_event_vector_table)
GTEXT(_machine_check_handler)
GTEXT(_general_exception_handler)
GTEXT(_trap0_handler)
GTEXT(_interrupt_handler)

/* Reset vector - must be aligned */
SECTION_FUNC(TEXT, __start)
    jump __reset_handler
    nop
    nop
    nop

/* Main reset handler for VM configuration */
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
    r29 = ##_interrupt_stack_end

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
    /* Jump to C initialization */
    call ##z_cstart

    /* Should never reach here */
    jump #_hang

_vm_version_error:
    r0 = #1
    jump #_fatal_error

_vmsetvec_error:
    r0 = #2
    jump #_fatal_error

/* Event vector table - must be 32-bit aligned */
SECTION_FUNC(event_vectors, _event_vector_table)
	.align 2

	/* Event 0: Reserved */
	jump ##_unhandled_event
	/* Event 1: Machine check */
	jump ##_machine_check_handler
	/* Event 2: General exception */
	jump ##_general_exception_handler
	/* Event 3: Reserved */
	jump ##_unhandled_event
	/* Event 4: Reserved */
	jump ##_unhandled_event
	/* Event 5: trap0 */
	jump ##_trap0_handler
	/* Event 6: Reserved */
	jump ##_unhandled_event
	/* Event 7: Interrupt */
	jump ##_interrupt_handler

/* Basic handlers for early boot */
SECTION_FUNC(TEXT, _unhandled_event)
	/* Save minimal context */
	r0 = #0xFF
	jump ##z_hexagon_fatal_error

SECTION_FUNC(TEXT, _machine_check_handler)
	r0 = #1
	jump ##z_hexagon_fatal_error

SECTION_FUNC(TEXT, _general_exception_handler)
	r0 = #2
	jump ##z_hexagon_fatal_error

SECTION_FUNC(TEXT, _trap0_handler)
	/* For now, just return */
	trap1(#0x1)  /* vmrte */

SECTION_FUNC(TEXT, _interrupt_handler)
	/* For now, just return */
	trap1(#0x1)  /* vmrte */

_fatal_error:
	/* r0 contains error code */
	jump #_hang

_hang:
	jump #_hang
