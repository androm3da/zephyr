/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>
#include <zephyr/arch/cpu.h>
#include <kernel_internal.h>

extern FUNC_NORETURN void z_cstart(void);
extern char _vector_table[];

/* Simple stubs for now */
static inline void soc_early_init(void) { }

/* Forward declare */
void z_hexagon_irq_init(void);
void z_hexagon_hvm_init(void);

/**
 * @brief Prepare to and run C code
 *
 * This routine prepares for the execution of and runs C code.
 */
void z_prep_c(void)
{
	/* Perform early SOC initialization */
	soc_early_init();

	/* Initialize BSS */
	z_bss_zero();

	/* Initialize HVM and interrupt system */
	z_hexagon_hvm_init();

#ifdef CONFIG_XIP
	/* Copy data from ROM to RAM */
	z_data_copy();
#endif

	/* Start kernel */
	z_cstart();
	CODE_UNREACHABLE;
}
