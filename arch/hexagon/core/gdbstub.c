/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon GDB stub implementation
 *
 * Provides remote debugging support via GDB protocol using Hexagon
 * debug facilities and VM operations.
 */

#include <zephyr/kernel.h>
#include <zephyr/debug/gdbstub.h>
#include <zephyr/arch/hexagon/gdbstub.h>
#include <zephyr/arch/hexagon/arch.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>

#ifdef CONFIG_GDBSTUB

/* GDB stub context */
static struct gdb_ctx hexagon_gdb_ctx;

/* Memory regions accessible by GDB */
static const struct gdb_mem_region gdb_mem_regions[] = {
	{.start = 0xc0000000, /* Default code start */
	 .end = 0xc0100000,   /* 1MB code region */
	 .attributes = GDB_MEM_REGION_RO,
	 .alignment = 4},
	{.start = 0xc0100000, /* Default data start */
	 .end = 0xc0200000,   /* 1MB data region */
	 .attributes = GDB_MEM_REGION_RW,
	 .alignment = 4},
	/* UART region */
	{.start = 0x10000000, .end = 0x10001000, .attributes = GDB_MEM_REGION_RW, .alignment = 4}};

#define GDB_MEM_REGION_ARRAY_SIZE ARRAY_SIZE(gdb_mem_regions)

/* Initialize GDB stub */
void arch_gdb_init(void)
{
	/* Clear context */
	memset(&hexagon_gdb_ctx, 0, sizeof(hexagon_gdb_ctx));

	/* Set up debug interrupt handler */
	/* TODO: Add actual interrupt setup when debug interrupts are implemented */

	printk("GDB stub initialized\n");
}

/* Continue execution */
void arch_gdb_continue(void)
{
	/* Clear stop flag */
	hexagon_gdb_ctx.stopped = false;

	/* Restore context and continue */
	/* TODO: Implement context restore when needed */
}

/* Single step */
void arch_gdb_step(void)
{
	/* Enable single step mode */
	/* TODO: Use VM operation for single step when available */

	/* Continue execution */
	arch_gdb_continue();
}

/* Get all registers */
size_t arch_gdb_reg_readall(struct gdb_ctx *ctx, uint8_t *buf, size_t buf_size)
{
	size_t ret;

	if (buf_size < (sizeof(ctx->regs) * 2)) {
		ret = 0;
	} else {
		ret = bin2hex((const uint8_t *)&(ctx->regs),
			      sizeof(ctx->regs), buf, buf_size);
	}

	return ret;
}

/* Set all registers */
size_t arch_gdb_reg_writeall(struct gdb_ctx *ctx, uint8_t *buf, size_t buf_size)
{
	size_t ret;

	if (buf_size < (sizeof(ctx->regs) * 2)) {
		ret = 0;
	} else {
		ret = hex2bin(buf, buf_size, (uint8_t *)&(ctx->regs), sizeof(ctx->regs));
	}

	return ret;
}

/* Read single register */
size_t arch_gdb_reg_readone(struct gdb_ctx *ctx, uint8_t *buf, size_t buf_size, uint32_t regno)
{
	size_t ret;

	if (regno >= GDB_HEXAGON_NUM_REGS || buf_size < 8) {
		ret = 0;
	} else {
		ret = bin2hex((const uint8_t *)&(ctx->regs[regno]), 4, buf, buf_size);
	}

	return ret;
}

/* Write single register */
size_t arch_gdb_reg_writeone(struct gdb_ctx *ctx, uint8_t *buf, size_t buf_size, uint32_t regno)
{
	size_t ret;

	if (regno >= GDB_HEXAGON_NUM_REGS || buf_size < 8) {
		ret = 0;
	} else {
		ret = hex2bin(buf, 8, (uint8_t *)&(ctx->regs[regno]), 4);
	}

	return ret;
}

/* Add breakpoint */
int arch_gdb_add_breakpoint(struct gdb_ctx *ctx, uint8_t type, uintptr_t addr, uint32_t kind)
{
	const static uint32_t brkpt = HEXAGON_BREAK_INSN;

	if (type != 0) {
		/* Only software breakpoints for now */
		return -1;
	}

	/* Find free slot */
	for (int i = 0; i < CONFIG_GDB_MAX_BREAKPOINTS; i++) {
		if (!ctx->breakpoints[i].active) {
			/* Save original instruction */
			ctx->breakpoints[i].addr = addr;
			ctx->breakpoints[i].saved_insn = *(uint32_t *)addr;

			memcpy((void *)addr, &brkpt, sizeof(brkpt));

			/* Flush cache if needed */
			/* TODO: Add cache flush when cache operations are implemented */

			ctx->breakpoints[i].active = true;
			return 0;
		}
	}

	return -1;
}

/* Remove breakpoint */
int arch_gdb_remove_breakpoint(struct gdb_ctx *ctx, uint8_t type, uintptr_t addr, uint32_t kind)
{
	if (type != 0) {
		return -1;
	}

	/* Find breakpoint */
	for (int i = 0; i < CONFIG_GDB_MAX_BREAKPOINTS; i++) {
		if (ctx->breakpoints[i].active && ctx->breakpoints[i].addr == addr) {
			/* Restore original instruction */
			*(uint32_t *)addr = ctx->breakpoints[i].saved_insn;

			/* Flush cache if needed */
			/* TODO: Add cache flush when cache operations are implemented */

			ctx->breakpoints[i].active = false;
			return 0;
		}
	}

	return -1;
}

/* GDB interrupt handler */
void gdb_isr(const void *arg)
{
	ARG_UNUSED(arg);

	/* Save current context */
	/* TODO: Implement context save when needed */

	/* Set exception reason */
	hexagon_gdb_ctx.exception = GDB_EXCEPTION_BREAKPOINT;
	hexagon_gdb_ctx.stopped = true;

	/* Enter GDB main loop */
	/* TODO: Integrate with Zephyr's GDB stub when available */
}

/* Memory access validation */
bool arch_gdb_memory_readable(uintptr_t addr, size_t size)
{
	/* Check if address range is in accessible regions */
	for (int i = 0; i < GDB_MEM_REGION_ARRAY_SIZE; i++) {
		if (addr >= gdb_mem_regions[i].start && (addr + size) <= gdb_mem_regions[i].end) {
			return true;
		}
	}

	return false;
}

bool arch_gdb_memory_writable(uintptr_t addr, size_t size)
{
	/* Check if address range is in writable regions */
	for (int i = 0; i < GDB_MEM_REGION_ARRAY_SIZE; i++) {
		if (addr >= gdb_mem_regions[i].start && (addr + size) <= gdb_mem_regions[i].end &&
		    (gdb_mem_regions[i].attributes & GDB_MEM_REGION_RW) == GDB_MEM_REGION_RW) {
			return true;
		}
	}

	return false;
}

#ifdef CONFIG_GDBSTUB_CUSTOM_BACKEND

/* Custom backend implementation using semihosting */
int z_gdb_backend_init(void)
{
	printk("GDB backend using semihosting initialized\n");
	return 0;
}

int z_gdb_backend_getchar(void)
{
	/* For now, return -1 to indicate no character available 
	 * In a real implementation, this would read from semihosting input */
	return -1;
}

void z_gdb_backend_putchar(int ch)
{
	/* Output character via semihosting (printk uses semihosting console) */
	char c = (char)ch;
	printk("%c", c);
}

#endif /* CONFIG_GDBSTUB_CUSTOM_BACKEND */

#endif /* CONFIG_GDBSTUB */
