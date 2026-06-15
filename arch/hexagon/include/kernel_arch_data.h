/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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
#include <zephyr/arch/hexagon/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file is intentionally minimal.  Hexagon does not require any
 * additional per-CPU or per-kernel data beyond what the generic kernel
 * provides.  It exists because the Zephyr build system unconditionally
 * includes <kernel_arch_data.h> from the kernel internals.
 */

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_DATA_H_ */
