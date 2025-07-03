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
    /* Enable interrupts */
    r0 = usr
    r0 = setbit(r0, #HEXAGON_USR_IE_BIT)
    usr = r0

    /* VM yield/wait */
    r0 = #HVM_TRAP0_VMWAIT
    trap0(#0)

    jumpr r31

SECTION_FUNC(TEXT, arch_cpu_atomic_idle)
    /* Save and get interrupt state */
    r2 = usr
    r0 = extractu(r2, #1, #HEXAGON_USR_IE_BIT)

    /* Enable interrupts */
    r2 = setbit(r2, #HEXAGON_USR_IE_BIT)
    usr = r2

    /* VM yield/wait */
    r1 = #HVM_TRAP0_VMWAIT
    trap0(#0)

    /* Restore interrupt state from r0 */
    jumpr r31
