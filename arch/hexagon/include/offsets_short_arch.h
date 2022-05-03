/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * based on arch/mips/include/offsets_short_arch.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_

#include <offsets.h>

#define _thread_offset_to_sp (___thread_t_callee_saved_OFFSET + ___callee_saved_t_sp_OFFSET)

#define _thread_offset_to_r16 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r16_OFFSET)

#define _thread_offset_to_r17 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r17_OFFSET)

#define _thread_offset_to_r18 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r18_OFFSET)

#define _thread_offset_to_r19 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r19_OFFSET)

#define _thread_offset_to_r20 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r20_OFFSET)

#define _thread_offset_to_r21 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r21_OFFSET)

#define _thread_offset_to_r22 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r22_OFFSET)

#define _thread_offset_to_r23 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r23_OFFSET)

#define _thread_offset_to_r24 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r24_OFFSET)

#define _thread_offset_to_r25 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r25_OFFSET)

#define _thread_offset_to_r26 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r26_OFFSET)

#define _thread_offset_to_r27 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r27_OFFSET)

#define _thread_offset_to_r28 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r28_OFFSET)

#define _thread_offset_to_r29 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r29_OFFSET)

#define _thread_offset_to_r30 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r30_OFFSET)

#define _thread_offset_to_r31 (___thread_t_callee_saved_OFFSET + ___callee_saved_t_r31_OFFSET)

#define _thread_offset_to_swap_return_value                                                        \
	(___thread_t_arch_OFFSET + ___thread_arch_t_swap_return_value_OFFSET)

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_ */
