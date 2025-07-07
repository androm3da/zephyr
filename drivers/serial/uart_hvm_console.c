/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hexagon Virtual Machine (HVM) console driver for Zephyr
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/arch/hexagon/hexagon_vm.h>

/* HVM console device structure */
struct hvm_console_data {
	struct k_spinlock lock;
};

static struct hvm_console_data hvm_console_data_0;

/**
 * @brief Write character to HVM console using angel console interface
 * @param c Character to write
 */
static void hvm_console_write_char(char c)
{
	struct angel_console_msg msg;
	static char buf[1];

	buf[0] = c;
	msg.op = ANGEL_CONSOLE_WRITE;
	msg.len = 1;
	msg.buf = buf;

	/* Use VMGETINFO with angel console type to perform I/O */
	__vmgetinfo(VM_INFO_TYPE_ANGEL_CONSOLE, &msg);
}

/**
 * @brief Poll out function for HVM console
 * @param dev Console device
 * @param c Character to output
 */
static void hvm_console_poll_out(const struct device *dev, unsigned char c)
{
	ARG_UNUSED(dev);

	hvm_console_write_char(c);

	/* Handle newline conversion */
	if (c == '\n') {
		hvm_console_write_char('\r');
	}
}

/**
 * @brief Poll in function for HVM console (not implemented)
 * @param dev Console device
 * @return -1 (no input available)
 */
static int hvm_console_poll_in(const struct device *dev, unsigned char *c)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(c);

	/* Read not implemented for HVM console */
	return -1;
}

/**
 * @brief Configure HVM console (no-op)
 * @param dev Console device
 * @param cfg Configuration
 * @return 0 (always success)
 */
static int hvm_console_configure(const struct device *dev,
				 const struct uart_config *cfg)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cfg);

	return 0;
}

/**
 * @brief Get HVM console configuration
 * @param dev Console device
 * @param cfg Configuration to fill
 * @return 0 (always success)
 */
static int hvm_console_config_get(const struct device *dev,
				  struct uart_config *cfg)
{
	ARG_UNUSED(dev);

	if (cfg) {
		cfg->baudrate = 115200;
		cfg->parity = UART_CFG_PARITY_NONE;
		cfg->stop_bits = UART_CFG_STOP_BITS_1;
		cfg->data_bits = UART_CFG_DATA_BITS_8;
		cfg->flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
	}

	return 0;
}

/**
 * @brief Initialize HVM console
 * @param dev Console device
 * @return 0 on success
 */
static int hvm_console_init(const struct device *dev)
{
	struct hvm_console_data *data = dev->data;
	long hvm_version;

	k_spinlock_init(&data->lock);

	/* Check if HVM is available */
	hvm_version = __vmversion();
	if (hvm_version <= 0) {
		return -ENODEV;
	}

	return 0;
}

/* HVM console driver API */
static const struct uart_driver_api hvm_console_driver_api = {
	.poll_in = hvm_console_poll_in,
	.poll_out = hvm_console_poll_out,
	.configure = hvm_console_configure,
	.config_get = hvm_console_config_get,
};

/* Define HVM console device */
DEVICE_DT_DEFINE(DT_NODELABEL(hvm_console),
		 hvm_console_init,
		 NULL,
		 &hvm_console_data_0,
		 NULL,
		 PRE_KERNEL_1,
		 CONFIG_SERIAL_INIT_PRIORITY,
		 &hvm_console_driver_api);
