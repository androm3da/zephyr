/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/*
 * Provides comprehensive interrupt and exception handling
 */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <hexagon_vm.h>

/* Event context structure offsets */
#define EVENT_CTX_R0_R1     0x00
#define EVENT_CTX_R2_R3     0x08
#define EVENT_CTX_R4_R5     0x10
#define EVENT_CTX_R6_R7     0x18
#define EVENT_CTX_R8_R9     0x20
#define EVENT_CTX_R10_R11   0x28
#define EVENT_CTX_R12_R13   0x30
#define EVENT_CTX_R14_R15   0x38
#define EVENT_CTX_PRED      0x40
#define EVENT_CTX_LR        0x44
#define EVENT_CTX_GUEST     0x48  /* g0-g3 */
#define EVENT_CTX_SIZE      0x58

GTEXT(z_hexagon_enhanced_machine_check)
GTEXT(z_hexagon_enhanced_exception)
GTEXT(z_hexagon_enhanced_interrupt)
GTEXT(z_hexagon_event_handler)

/* Common event entry macro */
.macro EVENT_ENTRY event_num
    /* Allocate space for event context */
    allocframe(#EVENT_CTX_SIZE)

    /* Save all volatile registers */
    {
        memd(r29+#EVENT_CTX_R0_R1) = r1:0
        memd(r29+#EVENT_CTX_R2_R3) = r3:2
    }
    {
        memd(r29+#EVENT_CTX_R4_R5) = r5:4
        memd(r29+#EVENT_CTX_R6_R7) = r7:6
    }
    {
        memd(r29+#EVENT_CTX_R8_R9) = r9:8
        memd(r29+#EVENT_CTX_R10_R11) = r11:10
    }
    {
        memd(r29+#EVENT_CTX_R12_R13) = r13:12
        memd(r29+#EVENT_CTX_R14_R15) = r15:14
    }

    /* Save predicate registers */
    {
        r0 = p3:0
        memw(r29+#EVENT_CTX_PRED) = r0
    }

    /* Save link register */
    {
        memw(r29+#EVENT_CTX_LR) = r31
    }

    /* Get guest registers with event info */
    /* g0-g3 contain event-specific information */
    r0 = g0
    r1 = g1
    {
        memw(r29+#EVENT_CTX_GUEST+0) = r0
        memw(r29+#EVENT_CTX_GUEST+4) = r1
    }
    r2 = g2
    r3 = g3
    {
        memw(r29+#EVENT_CTX_GUEST+8) = r2
        memw(r29+#EVENT_CTX_GUEST+12) = r3
    }

    /* Call C handler with event number and saved context */
    {
        r0 = #\event_num
        r1 = r29
    }
    {
        call ##z_hexagon_event_handler
    }
.endm

/* Common event exit macro */
.macro EVENT_EXIT event_id
    /* Check if we need to switch threads */
    {
        call ##z_arch_get_next_switch_handle
    }
    {
        p0 = cmp.eq(r0, #0)
        if (!p0) jump ##_event_switch_thread_\event_id
    }

_event_restore_\event_id:
    /* Restore guest registers */
    {
        r0 = memw(r29+#EVENT_CTX_GUEST+0)
        r1 = memw(r29+#EVENT_CTX_GUEST+4)
    }
    g0 = r0
    g1 = r1
    {
        r2 = memw(r29+#EVENT_CTX_GUEST+8)
        r3 = memw(r29+#EVENT_CTX_GUEST+12)
    }
    g2 = r2
    g3 = r3

    /* Restore link register */
    {
        r31 = memw(r29+#EVENT_CTX_LR)
    }

    /* Restore predicate registers */
    {
        r0 = memw(r29+#EVENT_CTX_PRED)
        p3:0 = r0
    }

    /* Restore volatile registers */
    {
        r15:14 = memd(r29+#EVENT_CTX_R14_R15)
        r13:12 = memd(r29+#EVENT_CTX_R12_R13)
    }
    {
        r11:10 = memd(r29+#EVENT_CTX_R10_R11)
        r9:8 = memd(r29+#EVENT_CTX_R8_R9)
    }
    {
        r7:6 = memd(r29+#EVENT_CTX_R6_R7)
        r5:4 = memd(r29+#EVENT_CTX_R4_R5)
    }
    {
        r3:2 = memd(r29+#EVENT_CTX_R2_R3)
        r1:0 = memd(r29+#EVENT_CTX_R0_R1)
    }

    /* Return from event */
    {
        deallocframe
    }
    {
        trap1(#HEXAGON_VM_vmrte)
    }

_event_switch_thread_\event_id:
    /* Perform context switch before returning */
    /* r0 contains next thread handle */
    {
        r1 = r0
        call ##k_current_get
    }
    {
        call ##z_hexagon_arch_switch
    }
    {
        jump ##_event_restore_\event_id
    }
.endm

/* Enhanced machine check handler */
SECTION_FUNC(TEXT, z_hexagon_enhanced_machine_check)
    EVENT_ENTRY 1
    EVENT_EXIT 1

/* Enhanced general exception handler */
SECTION_FUNC(TEXT, z_hexagon_enhanced_exception)
    EVENT_ENTRY 2
    EVENT_EXIT 2

/* Enhanced interrupt handler */
SECTION_FUNC(TEXT, z_hexagon_enhanced_interrupt)
    EVENT_ENTRY 7
    EVENT_EXIT 7

/* Helper function to get next switch handle (stub for now) */
SECTION_FUNC(TEXT, z_arch_get_next_switch_handle)
    {
        r0 = #0  /* No thread switch needed */
        jumpr r31
    }

