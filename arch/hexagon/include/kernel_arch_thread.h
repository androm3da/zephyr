/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_THREAD_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

/* Forward declarations - actual definitions are in zephyr/arch/hexagon/thread.h */
struct hexagon_callee_saved;
struct _thread_arch;
struct _callee_saved;

typedef struct _thread_arch _thread_arch_t;
typedef struct hexagon_callee_saved _callee_saved_t;

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_THREAD_H_ */
