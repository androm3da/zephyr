/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>
#include <zephyr/offsets.h>
#include <zephyr/arch/cpu.h>

GTEXT(z_hexagon_context_switch)
GTEXT(z_thread_entry_wrapper)

/* void z_hexagon_context_switch(struct k_thread *new, struct k_thread *old) */
SECTION_FUNC(TEXT, z_hexagon_context_switch)
    /* r0 = new thread, r1 = old thread */

    /* Save old thread context if not NULL */
    p0 = cmp.eq(r1, #0)
    if (p0) jump .Lrestore

    /* Save callee-saved registers */
    r2 = add(r1, #_thread_offset_to_callee_saved)
    memd(r2+#0) = r17:16
    memd(r2+#8) = r19:18
    memd(r2+#16) = r21:20
    memd(r2+#24) = r23:22
    memd(r2+#32) = r25:24
    memd(r2+#40) = r27:26
    memw(r2+#48) = r30  /* fp */
    memw(r2+#52) = r29  /* sp */
    memw(r2+#56) = r31  /* lr */

    /* Save predicate and user registers */
    r3 = p3:0
    memw(r2+#60) = r3
    r3 = usr
    memw(r2+#64) = r3

.Lrestore:
    /* Restore new thread context */
    r2 = add(r0, #_thread_offset_to_callee_saved)

    /* Restore callee-saved registers */
    r17:16 = memd(r2+#0)
    r19:18 = memd(r2+#8)
    r21:20 = memd(r2+#16)
    r23:22 = memd(r2+#24)
    r25:24 = memd(r2+#32)
    r27:26 = memd(r2+#40)
    r30 = memw(r2+#48)  /* fp */
    r29 = memw(r2+#52)  /* sp */
    r31 = memw(r2+#56)  /* lr */

    /* Restore predicate and user registers */
    r3 = memw(r2+#60)
    p3:0 = r3
    r3 = memw(r2+#64)
    usr = r3

    /* Update current thread pointer */
    r1 = ##_kernel
    memw(r1+#_kernel_offset_to_current) = r0

    jumpr r31

/* Thread entry wrapper */
SECTION_FUNC(TEXT, z_thread_entry_wrapper)
    /* r16 = entry point, r17 = p1, r18 = p2, r19 = p3 */
    r0 = r17
    r1 = r18
    r2 = r19
    callr r16

    /* Thread termination */
    jump z_thread_abort
