/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>
#include <zephyr/offsets.h>
#include <hexagon_vm.h>

GTEXT(arch_cpu_idle)
GTEXT(arch_cpu_atomic_idle)

SECTION_FUNC(TEXT, arch_cpu_idle)
    /* Enable interrupts using OR instead of setbit */
    r0 = usr
    r1 = #1
    r0 = or(r0, r1)
    usr = r0

    /* Simple loop instead of VM call */
    r0 = #100
cpu_idle_loop:
    r0 = add(r0, #-1)
    p0 = cmp.eq(r0, #0)
    if (!p0) jump cpu_idle_loop

    jumpr r31

SECTION_FUNC(TEXT, arch_cpu_atomic_idle)
    /* Save and get interrupt state */
    r2 = usr
    r1 = #1
    r0 = and(r2, r1)

    /* Enable interrupts using OR instead of setbit */
    r2 = or(r2, r1)
    usr = r2

    /* Simple loop instead of VM call */
    r1 = #100
cpu_atomic_idle_loop:
    r1 = add(r1, #-1)
    p0 = cmp.eq(r1, #0)
    if (!p0) jump cpu_atomic_idle_loop

    jumpr r31
