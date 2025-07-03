/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_DATA_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_DATA_H_

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/arch/cpu.h>
#include <kernel_arch_thread.h>

#ifndef _ASMLANGUAGE
#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/dlist.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hexagon exception stack frame */
struct arch_esf {
	uint32_t usr;    /* User status register */
	uint32_t elr;    /* Exception return address */
	uint32_t sp;     /* Stack pointer */
	uint32_t r0;     /* General registers */
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
};

/* Hexagon-specific kernel data */
struct _kernel_arch {
	/* Interrupt nesting level */
	uint32_t nested;
	/* Current IRQ being serviced */
	int curr_irq;
	/* Hardware thread ID */
	uint32_t thread_id;
};

typedef struct _kernel_arch _kernel_arch_t;

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_DATA_H_ */
