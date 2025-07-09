/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys_clock.h>

/* Simple stub implementation for Hexagon system timer */

uint32_t sys_clock_elapsed(void)
{
	/* For now, return 0 - no elapsed time */
	/* TODO: Implement proper Hexagon VM timer interface */
	return 0;
}

uint32_t sys_clock_cycle_get_32(void)
{
	/* TODO: Implement Hexagon cycle counter access */
	return 0;
}

uint64_t sys_clock_cycle_get_64(void)
{
	/* TODO: Implement Hexagon cycle counter access */
	return 0;
}

void sys_clock_set_timeout(int32_t ticks, bool idle)
{
	/* TODO: Implement Hexagon timer timeout */
}

void sys_clock_disable(void)
{
	/* TODO: Implement timer disable */
}

static int hexagon_timer_init(void)
{
	/* TODO: Initialize Hexagon VM timer interface */
	return 0;
}

SYS_INIT(hexagon_timer_init, PRE_KERNEL_2, CONFIG_SYSTEM_CLOCK_INIT_PRIORITY);
