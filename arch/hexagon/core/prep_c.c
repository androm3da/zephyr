/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>
#include <zephyr/arch/cpu.h>
#include <hexagon_vm.h>

extern FUNC_NORETURN void z_cstart(void);
extern void _vector_table[];
extern void soc_early_init(void);
extern void z_hexagon_irq_init(void);

/**
 * @brief Prepare to and run C code
 *
 * This routine prepares for the execution of and runs C code.
 */
void z_prep_c(void)
{
	/* Perform early SOC initialization */
	soc_early_init();

	/* Initialize VM interface */
	hvm_set_vector(_vector_table);

	/* Enable interrupts at VM level */
	hvm_enable_interrupts();

	/* Initialize BSS */
	z_bss_zero();

	/* Initialize interrupt system */
	z_hexagon_irq_init();

#ifdef CONFIG_XIP
	/* Copy data from ROM to RAM */
	z_data_copy();
#endif

	/* Start kernel */
	z_cstart();
	CODE_UNREACHABLE;
}
