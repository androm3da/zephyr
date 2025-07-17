/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_H_

#include <zephyr/devicetree.h>
#include <zephyr/arch/hexagon/thread.h>
#include <zephyr/arch/hexagon/exception.h>
#include <zephyr/arch/common/sys_bitops.h>
#include <zephyr/arch/common/ffs.h>
#include <zephyr/arch/common/sys_io.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel/mm.h>
#include <hexagon_vm.h>
#include <irq.h>
#include <zephyr/sys/printk.h>

#if defined(CONFIG_GDBSTUB)
#include <zephyr/arch/hexagon/gdbstub.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Event types */
#define HEXAGON_EVENT_RESERVED1         0
#define HEXAGON_EVENT_MACHINE_CHECK     1
#define HEXAGON_EVENT_GENERAL_EXCEPTION 2
#define HEXAGON_EVENT_TRAP0             5
#define HEXAGON_EVENT_INTERRUPT         7

/* Stack alignment requirement */
#define ARCH_STACK_PTR_ALIGN 8

/* Architecture-specific kernel initialization */
static inline void arch_kernel_init(void)
{
	/* Initialize VM interface */
}

extern uint32_t sys_clock_cycle_get_32(void);

static inline uint32_t arch_k_cycle_get_32(void)
{
	return sys_clock_cycle_get_32();
}

extern uint64_t sys_clock_cycle_get_64(void);

static inline uint64_t arch_k_cycle_get_64(void)
{
	return sys_clock_cycle_get_64();
}

/* No-op instruction */
static inline void arch_nop(void)
{
	__asm__ volatile("nop");
}

/* Check if we're in an interrupt context */
static inline bool arch_is_in_isr(void)
{
	return false; /* TODO: Implement based on context */
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_H_ */
