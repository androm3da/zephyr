/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

/* Hexagon semihosting SYS_WRITE */
#define SEMIHOST_SYS_WRITE 0x05

static int semihost_write(const char *buf, size_t len)
{
	register int r0 asm("r0") = SEMIHOST_SYS_WRITE;
	register int r1 asm("r1") = 1; /* stdout */
	register const char *r2 asm("r2") = buf;
	register size_t r3 asm("r3") = len;

	asm volatile(
		"trap0(#0xdb)"
		: "+r"(r0)
		: "r"(r1), "r"(r2), "r"(r3)
		: "memory"
	);

	return r0;
}

static void uart_semihost_poll_out(const struct device *dev, unsigned char c)
{
	char buf = c;
	semihost_write(&buf, 1);
}

static int uart_semihost_init(const struct device *dev)
{
	return 0;
}

static const struct uart_driver_api uart_semihost_driver_api = {
	.poll_out = uart_semihost_poll_out,
};

DEVICE_DT_INST_DEFINE(0, uart_semihost_init, NULL,
		      NULL, NULL,
		      PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		      &uart_semihost_driver_api);
