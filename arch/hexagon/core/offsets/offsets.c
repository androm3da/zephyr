/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Hexagon kernel structure member offset definition file
 *
 * This module is responsible for the generation of the absolute symbols whose
 * value represents the member offsets for various Hexagon kernel structures.
 */

#include <zephyr/kernel.h>
#include <kernel_arch_data.h>
#include <gen_offset.h>
#include <kernel_offsets.h>

/* struct _callee_saved member offsets */
GEN_OFFSET_SYM(_callee_saved_t, r16);
GEN_OFFSET_SYM(_callee_saved_t, r17);
GEN_OFFSET_SYM(_callee_saved_t, r18);
GEN_OFFSET_SYM(_callee_saved_t, r19);
GEN_OFFSET_SYM(_callee_saved_t, r20);
GEN_OFFSET_SYM(_callee_saved_t, r21);
GEN_OFFSET_SYM(_callee_saved_t, r22);
GEN_OFFSET_SYM(_callee_saved_t, r23);
GEN_OFFSET_SYM(_callee_saved_t, r24);
GEN_OFFSET_SYM(_callee_saved_t, r25);
GEN_OFFSET_SYM(_callee_saved_t, r26);
GEN_OFFSET_SYM(_callee_saved_t, r27);
GEN_OFFSET_SYM(_callee_saved_t, fp);
GEN_OFFSET_SYM(_callee_saved_t, sp);
GEN_OFFSET_SYM(_callee_saved_t, lr);
GEN_OFFSET_SYM(_callee_saved_t, p3_0);
GEN_OFFSET_SYM(_callee_saved_t, usr);

GEN_ABSOLUTE_SYM(__callee_saved_t_SIZEOF, ROUND_UP(sizeof(_callee_saved_t), ARCH_STACK_PTR_ALIGN));

GEN_ABS_SYM_END
