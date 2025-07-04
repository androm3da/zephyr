/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_STRUCTS_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_STRUCTS_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

/* Per CPU architecture specifics for Hexagon */
struct _cpu_arch {
	/* Hardware thread ID */
	uint32_t thread_id;
	/* Exception nesting level */
	uint32_t nested;
	/* Current exception vector */
	uint32_t curr_vector;
};

/* typedefs to be used with GEN_OFFSET_SYM(), etc. */
typedef struct _cpu_arch _cpu_arch_t;

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_STRUCTS_H_ */