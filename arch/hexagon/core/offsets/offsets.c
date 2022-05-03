/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Hexagon kernel structure member offset definition file
 *
 * This module is responsible for the generation of the absolute symbols whose
 * value represents the member offsets for various Hexagon kernel structures.
 *
 * All of the absolute symbols defined by this module will be present in the
 * final kernel ELF image (due to the linker's reference to the _OffsetAbsSyms
 * symbol).
 */

#ifndef _HEXAGON_OFFSETS_INC_
#define _HEXAGON_OFFSETS_INC_

#if 0
#include <kernel.h>
#include <kernel_arch_data.h>
#include <kernel_offsets.h>

GEN_OFFSET_SYM(_thread_arch_t, prio);

GEN_ABSOLUTE_SYM(___basic_sf_t_SIZEOF, sizeof(_basic_sf_t));

GEN_ABSOLUTE_SYM(___esf_t_SIZEOF, sizeof(_esf_t));

#if defined(CONFIG_THREAD_STACK_INFO)
GEN_OFFSET_SYM(_thread_stack_info_t, start);

GEN_ABSOLUTE_SYM(___thread_stack_info_t_SIZEOF,
	 sizeof(struct _thread_stack_info));
#endif

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

GEN_OFFSET_SYM(_callee_saved_t, r29);
GEN_OFFSET_SYM(_callee_saved_t, r30);
GEN_OFFSET_SYM(_callee_saved_t, r31);

#if defined(CONFIG_HVX)
GEN_OFFSET_SYM(_callee_saved_t, v0);
#error "fixme - hexagon fill this in"
#endif
#endif

#endif /* _HEXAGON_OFFSETS_INC_ */
