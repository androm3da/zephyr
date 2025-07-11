/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>
#include <zephyr/arch/hexagon/vm_ops.h>

extern FUNC_NORETURN void z_cstart(void);

/* Forward declaration for fatal error handler */
void z_hexagon_fatal_error(unsigned int reason);

/* Architecture-specific early initialization */
void z_hexagon_early_init(void)
{
	/* Any early platform-specific initialization */
}

void z_prep_c(void)
{
	/* Early architecture initialization */
	z_hexagon_early_init();
}
