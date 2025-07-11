/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>

GTEXT(z_thread_entry_wrapper)
GTEXT(z_hexagon_thread_entry)

/*
 * Thread entry wrapper
 * This is the first code executed by a new thread
 */
SECTION_FUNC(TEXT, z_thread_entry_wrapper)
    /* Stack frame already set up by arch_new_thread */
    /* Load parameters from stack */
    {
        r0 = memw(r29+#0)  /* entry point */
        r1 = memw(r29+#4)  /* parameter1 */
    }
    {
        r2 = memw(r29+#8)  /* parameter2 */
        r3 = memw(r29+#12) /* parameter3 */
    }

    /* Adjust stack to final position */
    {
        r29 = add(r29, #16)
    }

    /* Jump to C entry wrapper */
    {
        jump ##z_hexagon_thread_entry
    }
