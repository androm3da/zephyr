/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_

/* Architecture-specific short offsets for Hexagon */

/* Thread control block offsets */
#define _thread_offset_to_sp \
	(___thread_t_callee_saved_OFFSET + ___callee_saved_t_sp_OFFSET)

#define _thread_offset_to_thread_state \
	(___thread_t_thread_state_OFFSET)

/* Exception stack frame offsets */
#define _esf_offset_to_r0 \
	(___esf_t_r0_OFFSET)

#define _esf_offset_to_elr \
	(___esf_t_elr_OFFSET)

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_OFFSETS_SHORT_ARCH_H_ */