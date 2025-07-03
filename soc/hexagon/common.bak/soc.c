/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <soc.h>
#include <zephyr/arch/cpu.h>

/**
 * @brief Perform basic hardware initialization at boot.
 *
 * This needs to be run from the very beginning.
 */
void soc_early_init(void)
{
    /* Initialize cache if available */
#ifdef CONFIG_HEXAGON_HAS_L2_CACHE
    /* L2 cache initialization would go here */
#endif
}

/**
 * @brief Perform basic hardware initialization
 */
static int hexagon_soc_init(const struct device *dev)
{
    ARG_UNUSED(dev);

    /* Initialize hardware features */
#ifdef CONFIG_HEXAGON_HAS_HVX
    /* HVX initialization would go here */
#endif

    return 0;
}

SYS_INIT(hexagon_soc_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
