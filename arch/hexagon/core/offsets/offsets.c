/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>
#include <kernel_offsets.h>

/* Hexagon-specific offsets */
GEN_OFFSET_SYM(_thread_arch_t, callee_saved);
GEN_OFFSET_SYM(_thread_arch_t, swap_return_value);
GEN_OFFSET_SYM(_thread_arch_t, stack_top);

/* Callee-saved register offsets */
GEN_OFFSET_SYM(hexagon_callee_saved, r16);
GEN_OFFSET_SYM(hexagon_callee_saved, r17);
GEN_OFFSET_SYM(hexagon_callee_saved, r18);
GEN_OFFSET_SYM(hexagon_callee_saved, r19);
GEN_OFFSET_SYM(hexagon_callee_saved, r20);
GEN_OFFSET_SYM(hexagon_callee_saved, r21);
GEN_OFFSET_SYM(hexagon_callee_saved, r22);
GEN_OFFSET_SYM(hexagon_callee_saved, r23);
GEN_OFFSET_SYM(hexagon_callee_saved, r24);
GEN_OFFSET_SYM(hexagon_callee_saved, r25);
GEN_OFFSET_SYM(hexagon_callee_saved, r26);
GEN_OFFSET_SYM(hexagon_callee_saved, r27);
GEN_OFFSET_SYM(hexagon_callee_saved, fp);
GEN_OFFSET_SYM(hexagon_callee_saved, sp);
GEN_OFFSET_SYM(hexagon_callee_saved, lr);
GEN_OFFSET_SYM(hexagon_callee_saved, p3_0);
GEN_OFFSET_SYM(hexagon_callee_saved, usr);

GEN_ABSOLUTE_SYM(__hexagon_callee_saved_t_SIZEOF, sizeof(struct hexagon_callee_saved));
