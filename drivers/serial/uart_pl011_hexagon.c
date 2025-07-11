/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/irq.h>

/* PL011 UART registers */
#define PL011_DR    0x000 /* Data register */
#define PL011_RSR   0x004 /* Receive status register */
#define PL011_ECR   0x004 /* Error clear register */
#define PL011_FR    0x018 /* Flag register */
#define PL011_ILPR  0x020 /* IrDA low-power counter */
#define PL011_IBRD  0x024 /* Integer baud rate divisor */
#define PL011_FBRD  0x028 /* Fractional baud rate divisor */
#define PL011_LCR_H 0x02C /* Line control register */
#define PL011_CR    0x030 /* Control register */
#define PL011_IFLS  0x034 /* Interrupt FIFO level select */
#define PL011_IMSC  0x038 /* Interrupt mask set/clear */
#define PL011_RIS   0x03C /* Raw interrupt status */
#define PL011_MIS   0x040 /* Masked interrupt status */
#define PL011_ICR   0x044 /* Interrupt clear register */

/* Flag register bits */
#define PL011_FR_TXFF BIT(5) /* Transmit FIFO full */
#define PL011_FR_RXFE BIT(4) /* Receive FIFO empty */
#define PL011_FR_BUSY BIT(3) /* UART busy */

/* Control register bits */
#define PL011_CR_UARTEN BIT(0) /* UART enable */
#define PL011_CR_TXE    BIT(8) /* Transmit enable */
#define PL011_CR_RXE    BIT(9) /* Receive enable */

/* Line control register bits */
#define PL011_LCR_H_FEN    BIT(4)   /* Enable FIFOs */
#define PL011_LCR_H_WLEN_8 (3 << 5) /* 8 bits word length */

/* Interrupt bits */
#define PL011_IMSC_RXIM BIT(4) /* Receive interrupt mask */
#define PL011_IMSC_TXIM BIT(5) /* Transmit interrupt mask */

/* Device configuration */
struct uart_pl011_config {
	mm_reg_t base;
	uint32_t irq_num;
	uint32_t irq_prio;
	uint32_t baud_rate;
};

/* Device runtime data */
struct uart_pl011_data {
	struct uart_config cfg;
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	uart_irq_callback_user_data_t cb;
	void *cb_data;
#endif
};

/* Helper functions */
static inline uint32_t pl011_read(const struct device *dev, uint32_t reg)
{
	const struct uart_pl011_config *config = dev->config;
	return sys_read32(config->base + reg);
}

static inline void pl011_write(const struct device *dev, uint32_t reg, uint32_t val)
{
	const struct uart_pl011_config *config = dev->config;
	sys_write32(val, config->base + reg);
}

/* UART API implementation */
static int uart_pl011_poll_in(const struct device *dev, unsigned char *c)
{
	if (pl011_read(dev, PL011_FR) & PL011_FR_RXFE) {
		return -1; /* No data available */
	}

	*c = (unsigned char)pl011_read(dev, PL011_DR);
	return 0;
}

static void uart_pl011_poll_out(const struct device *dev, unsigned char c)
{
	/* Wait for transmit FIFO not full */
	while (pl011_read(dev, PL011_FR) & PL011_FR_TXFF) {
		/* Busy wait */
	}

	pl011_write(dev, PL011_DR, c);
}

static int uart_pl011_err_check(const struct device *dev)
{
	uint32_t rsr = pl011_read(dev, PL011_RSR);
	int errors = 0;

	if (rsr & BIT(0)) {
		errors |= UART_ERROR_FRAMING;
	}
	if (rsr & BIT(1)) {
		errors |= UART_ERROR_PARITY;
	}
	if (rsr & BIT(2)) {
		errors |= UART_ERROR_BREAK;
	}
	if (rsr & BIT(3)) {
		errors |= UART_ERROR_OVERRUN;
	}

	/* Clear errors */
	if (errors) {
		pl011_write(dev, PL011_ECR, 0);
	}

	return errors;
}

#ifdef CONFIG_UART_INTERRUPT_DRIVEN

static int uart_pl011_fifo_fill(const struct device *dev, const uint8_t *tx_data, int len)
{
	int num_sent = 0;

	while (num_sent < len) {
		if (pl011_read(dev, PL011_FR) & PL011_FR_TXFF) {
			break; /* FIFO full */
		}
		pl011_write(dev, PL011_DR, tx_data[num_sent++]);
	}

	return num_sent;
}

static int uart_pl011_fifo_read(const struct device *dev, uint8_t *rx_data, const int len)
{
	int num_read = 0;

	while (num_read < len) {
		if (pl011_read(dev, PL011_FR) & PL011_FR_RXFE) {
			break; /* FIFO empty */
		}
		rx_data[num_read++] = (uint8_t)pl011_read(dev, PL011_DR);
	}

	return num_read;
}

static void uart_pl011_irq_tx_enable(const struct device *dev)
{
	uint32_t imsc = pl011_read(dev, PL011_IMSC);
	pl011_write(dev, PL011_IMSC, imsc | PL011_IMSC_TXIM);
}

static void uart_pl011_irq_tx_disable(const struct device *dev)
{
	uint32_t imsc = pl011_read(dev, PL011_IMSC);
	pl011_write(dev, PL011_IMSC, imsc & ~PL011_IMSC_TXIM);
}

static int uart_pl011_irq_tx_ready(const struct device *dev)
{
	return !(pl011_read(dev, PL011_FR) & PL011_FR_TXFF);
}

static void uart_pl011_irq_rx_enable(const struct device *dev)
{
	uint32_t imsc = pl011_read(dev, PL011_IMSC);
	pl011_write(dev, PL011_IMSC, imsc | PL011_IMSC_RXIM);
}

static void uart_pl011_irq_rx_disable(const struct device *dev)
{
	uint32_t imsc = pl011_read(dev, PL011_IMSC);
	pl011_write(dev, PL011_IMSC, imsc & ~PL011_IMSC_RXIM);
}

static int uart_pl011_irq_rx_ready(const struct device *dev)
{
	return !(pl011_read(dev, PL011_FR) & PL011_FR_RXFE);
}

static void uart_pl011_irq_callback_set(const struct device *dev, uart_irq_callback_user_data_t cb,
					void *cb_data)
{
	struct uart_pl011_data *data = dev->data;
	data->cb = cb;
	data->cb_data = cb_data;
}

static void uart_pl011_isr(const void *arg)
{
	const struct device *dev = arg;
	struct uart_pl011_data *data = dev->data;

	if (data->cb) {
		data->cb(dev, data->cb_data);
	}

	/* Clear interrupts */
	pl011_write(dev, PL011_ICR, 0x7FF);
}

#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

/* Driver initialization - kept simple to avoid interfering with semihosting */
static int uart_pl011_init(const struct device *dev)
{
	/* For now, we'll keep this minimal since semihosting works well */
	/* This allows the driver to exist without breaking semihosting */
	printk("PL011 UART driver initialized (semihosting mode)\n");
	return 0;
}

/* UART driver API */
static const struct uart_driver_api uart_pl011_api = {
	.poll_in = uart_pl011_poll_in,
	.poll_out = uart_pl011_poll_out,
	.err_check = uart_pl011_err_check,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.fifo_fill = uart_pl011_fifo_fill,
	.fifo_read = uart_pl011_fifo_read,
	.irq_tx_enable = uart_pl011_irq_tx_enable,
	.irq_tx_disable = uart_pl011_irq_tx_disable,
	.irq_tx_ready = uart_pl011_irq_tx_ready,
	.irq_rx_enable = uart_pl011_irq_rx_enable,
	.irq_rx_disable = uart_pl011_irq_rx_disable,
	.irq_rx_ready = uart_pl011_irq_rx_ready,
	.irq_callback_set = uart_pl011_irq_callback_set,
#endif
};

#ifdef CONFIG_UART_PL011_HEXAGON

/* Device instance */
static const struct uart_pl011_config uart_pl011_config_0 = {
	.base = 0x10000000, /* PL011 UART base address */
	.irq_num = 15,      /* Hexagon VM IRQ number */
	.irq_prio = 1,
	.baud_rate = 115200,
};

static struct uart_pl011_data uart_pl011_data_0 = {
	.cfg =
		{
			.baudrate = 115200,
			.parity = UART_CFG_PARITY_NONE,
			.stop_bits = UART_CFG_STOP_BITS_1,
			.data_bits = UART_CFG_DATA_BITS_8,
			.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
		},
};

/* Don't use SYS_INIT to avoid semihosting issues */
DEVICE_DEFINE(uart_pl011_0, "uart_pl011_0", uart_pl011_init, NULL, &uart_pl011_data_0,
	      &uart_pl011_config_0, POST_KERNEL, CONFIG_SERIAL_INIT_PRIORITY, &uart_pl011_api);

#endif /* CONFIG_UART_PL011_HEXAGON */
