/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>

GTEXT(__start)
GTEXT(_machine_check_handler)
GTEXT(_general_exception_handler)
GTEXT(_trap0_handler)
GTEXT(_interrupt_handler)

/* Event vector table - must be 64-byte aligned */
SECTION_FUNC(event_vectors, __start)
	.align 64

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
	trap1(#1)

SECTION_FUNC(TEXT, _interrupt_handler)
	/* For now, just return */
	trap1(#1)
