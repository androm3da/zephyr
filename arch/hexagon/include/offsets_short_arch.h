/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_

#include <zephyr/offsets.h>

/* Hexagon-specific offset shorts */
#define _thread_offset_to_r16 \
    (_thread_offset_to_callee_saved + _hexagon_callee_saved_offset_to_r16)
#define _thread_offset_to_r17 \
    (_thread_offset_to_callee_saved + _hexagon_callee_saved_offset_to_r17)
#define _thread_offset_to_sp \
    (_thread_offset_to_callee_saved + _hexagon_callee_saved_offset_to_sp)
#define _thread_offset_to_lr \
    (_thread_offset_to_callee_saved + _hexagon_callee_saved_offset_to_lr)

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_ */
