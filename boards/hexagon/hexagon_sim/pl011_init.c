/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>

/* Placeholder for PL011 initialization if needed */
static int pl011_init(const struct device *dev)
{
    ARG_UNUSED(dev);
    return 0;
}

SYS_INIT(pl011_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
