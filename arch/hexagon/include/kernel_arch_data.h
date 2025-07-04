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
#include <zephyr/types.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/dlist.h>

#ifdef __cplusplus
extern "C" {
#endif

/* arch_esf is defined in zephyr/arch/hexagon/arch.h */

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
