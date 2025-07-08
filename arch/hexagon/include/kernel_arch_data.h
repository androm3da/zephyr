/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_DATA_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_DATA_H_

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/kernel.h>

#ifndef _ASMLANGUAGE
#include <zephyr/kernel_structs.h>
#include <zephyr/types.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/dlist.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hexagon-specific thread structure additions */
struct _thread_arch {
	uint32_t swap_return_value;
	uint32_t vm_event_info[4]; /* g0-g3 for event handling */
};

typedef struct _thread_arch _thread_arch_t;

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_DATA_H_ */
