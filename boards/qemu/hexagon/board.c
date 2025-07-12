/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief QEMU Hexagon board initialization
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>

/* Board-specific initialization */
static int qemu_hexagon_init(void)
{
	/* Initialize board-specific features */
	printk("QEMU Hexagon board initialized\n");
	printk("Hexagon VM support: %s\n", IS_ENABLED(CONFIG_HEXAGON_VM) ? "enabled" : "disabled");
	printk("Hardware threads: %s\n",
	       IS_ENABLED(CONFIG_HEXAGON_HW_THREADS) ? "enabled" : "disabled");
	printk("HVX: %s\n", IS_ENABLED(CONFIG_HEXAGON_HVX) ? "enabled" : "disabled");
	printk("Power management: %s\n", IS_ENABLED(CONFIG_PM) ? "enabled" : "disabled");

	/* Any additional board-specific setup */
	return 0;
}

SYS_INIT(qemu_hexagon_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
