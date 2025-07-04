/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/offsets.h>

GTEXT(__start)
GTEXT(z_prep_c)

SECTION_SUBSEC_FUNC(TEXT, _TEXT_SECTION_NAME_2, __start)
    /* Clear registers */
    r0 = #0
    r1 = #0
    r2 = #0
    r3 = #0
    r4 = #0
    r5 = #0

    /* Set up stack pointer */
    r29 = ##(_interrupt_stack + CONFIG_ISR_STACK_SIZE)

    /* Clear frame pointer */
    r30 = #0

    /* Disable interrupts initially using AND instead of clrbit */
    r0 = usr
    r1 = #0xFFFFFFFE  /* ~(1 << HEXAGON_USR_IE_BIT) */
    r0 = and(r0, r1)
    usr = r0

    /* Jump to C preparation code */
    jump z_prep_c
