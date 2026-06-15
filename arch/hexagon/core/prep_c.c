/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <kernel_internal.h>

extern void hexagon_mmu_init(void);

/**
 * z_prep_c - Architecture-specific C entry point.
 *
 * Called from the reset handler in hvm_event_vectors.S after BSS has
 * been cleared.  Sets up page tables and switches to table-based MMU
 * translation, then hands off to the kernel via z_cstart().
 */
FUNC_NORETURN void z_prep_c(void)
{
	hexagon_mmu_init();
	z_cstart();
	CODE_UNREACHABLE;
}
