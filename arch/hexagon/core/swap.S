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
    r2 = add(r1, #___thread_t_callee_saved_OFFSET)
    memw(r2+#0) = r16
    memw(r2+#4) = r17
    memw(r2+#8) = r18
    memw(r2+#12) = r19
    memw(r2+#16) = r20
    memw(r2+#20) = r21
    memw(r2+#24) = r22
    memw(r2+#28) = r23
    memw(r2+#32) = r24
    memw(r2+#36) = r25
    memw(r2+#40) = r26
    memw(r2+#44) = r27
    memw(r2+#48) = r30  /* fp */
    memw(r2+#52) = r29  /* sp */
    memw(r2+#56) = r31  /* lr */

    /* Save user registers */
    r3 = usr
    memw(r2+#60) = r3

.Lrestore:
    /* Restore new thread context */
    r2 = add(r0, #___thread_t_callee_saved_OFFSET)

    /* Restore callee-saved registers */
    r16 = memw(r2+#0)
    r17 = memw(r2+#4)
    r18 = memw(r2+#8)
    r19 = memw(r2+#12)
    r20 = memw(r2+#16)
    r21 = memw(r2+#20)
    r22 = memw(r2+#24)
    r23 = memw(r2+#28)
    r24 = memw(r2+#32)
    r25 = memw(r2+#36)
    r26 = memw(r2+#40)
    r27 = memw(r2+#44)
    r30 = memw(r2+#48)  /* fp */
    r29 = memw(r2+#52)  /* sp */
    r31 = memw(r2+#56)  /* lr */

    /* Restore user registers */
    r3 = memw(r2+#60)
    usr = r3

    /* Update current thread pointer */
    r1 = ##_kernel
    r1 = add(r1, #___kernel_t_cpus_OFFSET)  /* Point to cpus[0] */
    memw(r1+#___cpu_t_current_OFFSET) = r0  /* Set cpus[0].current */

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
