/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/arch/hexagon/vm_ops.h>

GTEXT(__start)
GTEXT(__reset_handler)
GTEXT(_machine_check_handler)
GTEXT(_general_exception_handler)
GTEXT(_trap0_handler)
GTEXT(_interrupt_handler)

/* Reset vector - must be aligned */
SECTION_FUNC(reset, __start)
	.align 2
	/* Event 0: Reserved */
	jump #__reset_handler
	/* Event 1: Machine check */
	jump #_machine_check_handler
	/* Event 2: General exception */
	jump #_general_exception_handler
	/* Event 3: Reserved */
	jump #_unhandled_event
	/* Event 4: Reserved */
	jump #_unhandled_event
	/* Event 5: trap0 */
	jump #_trap0_handler
	/* Event 6: Reserved */
	jump #_unhandled_event
	/* Event 7: Interrupt */
	jump #_interrupt_handler


/* Main reset handler for VM configuration */
SECTION_FUNC(TEXT, __reset_handler)
    /* Set event vector base */
    r0 = ##__start
    trap1(#HEXAGON_VM_vmsetvec)
    p0 = cmp.eq(r0, #0)
    if (!p0) jump #_vmsetvec_error

    /* Disable interrupts */
    r0 = #0
    trap1(#HEXAGON_VM_vmsetie)

    /* Get VM version for compatibility check */
    r0 = #CONFIG_HEXAGON_VM_VERSION
    trap1(#HEXAGON_VM_vmversion)
    p0 = cmp.eq(r0, #CONFIG_HEXAGON_VM_VERSION)
    if (!p0) jump #_vm_version_error

    /* Set up MMU mappings for Zephyr image */
    /* Map 0xc0000000 to 0xc0000000 + 64MB */
    r0 = #0xc0000000      /* Virtual address */
    r1 = #0xc0000000      /* Physical address */
    r2 = #0x4000000       /* Size: 64MB */
    r3 = #0x3F            /* PTE: valid, read, write, execute, cache */
    trap1(#HEXAGON_VM_vmnewmap)
    p0 = cmp.eq(r0, #0)
    if (!p0) jump #_mmu_error

    /* Set up initial stack pointer */
    r29 = ##_interrupt_stack_end

    /* Clear frame pointer */
    r30 = #0

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

_mmu_error:
    r0 = #3
    jump #_fatal_error

/* Event vector table - must be 32-bit aligned */
SECTION_FUNC(event_vectors, _event_vector_table)
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
	/* Basic system call handler */
	/* r0 contains syscall number, r1-r3 contain arguments */
	
	/* Check syscall number */
	p0 = cmp.eq(r0, #3)  /* SYS_write */
	if (p0) jump #_handle_write
	
	/* For unhandled system calls, return 0 */
	r0 = #0
	trap1(#HEXAGON_VM_vmrte)

_handle_write:
	/* For write syscall, just return the count (r2) to simulate success */
	r0 = r2  /* Return bytes written = requested count */
	trap1(#HEXAGON_VM_vmrte)

SECTION_FUNC(TEXT, _interrupt_handler)
	/* For now, just return */
	trap1(#HEXAGON_VM_vmrte)

_fatal_error:
	/* r0 contains error code */
	stop(r0)
	jump #_hang

_hang:
	jump #_hang
