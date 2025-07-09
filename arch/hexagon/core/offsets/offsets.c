/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/arch/exception.h>
#include <zephyr/kernel.h>
#include <kernel_arch_data.h>
#include <gen_offset.h>

GEN_ABS_SYM_BEGIN(offsets)

GEN_OFFSET_SYM(_thread_arch_t, swap_return_value);
GEN_OFFSET_SYM(_thread_arch_t, vm_event_info);

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
GEN_OFFSET_SYM(_callee_saved_t, r29_fp);
GEN_OFFSET_SYM(_callee_saved_t, r30_sp);
GEN_OFFSET_SYM(_callee_saved_t, r31_lr);

GEN_OFFSET_STRUCT(arch_esf, r0);
GEN_OFFSET_STRUCT(arch_esf, r1);
GEN_OFFSET_STRUCT(arch_esf, r2);
GEN_OFFSET_STRUCT(arch_esf, r3);
GEN_OFFSET_STRUCT(arch_esf, r4);
GEN_OFFSET_STRUCT(arch_esf, r5);
GEN_OFFSET_STRUCT(arch_esf, r6);
GEN_OFFSET_STRUCT(arch_esf, r7);
GEN_OFFSET_STRUCT(arch_esf, r8);
GEN_OFFSET_STRUCT(arch_esf, r9);
GEN_OFFSET_STRUCT(arch_esf, r10);
GEN_OFFSET_STRUCT(arch_esf, r11);
GEN_OFFSET_STRUCT(arch_esf, r12);
GEN_OFFSET_STRUCT(arch_esf, r13);
GEN_OFFSET_STRUCT(arch_esf, r14);
GEN_OFFSET_STRUCT(arch_esf, r15);
GEN_OFFSET_STRUCT(arch_esf, r16);
GEN_OFFSET_STRUCT(arch_esf, r17);
GEN_OFFSET_STRUCT(arch_esf, r18);
GEN_OFFSET_STRUCT(arch_esf, r19);
GEN_OFFSET_STRUCT(arch_esf, r20);
GEN_OFFSET_STRUCT(arch_esf, r21);
GEN_OFFSET_STRUCT(arch_esf, r22);
GEN_OFFSET_STRUCT(arch_esf, r23);
GEN_OFFSET_STRUCT(arch_esf, r24);
GEN_OFFSET_STRUCT(arch_esf, r25);
GEN_OFFSET_STRUCT(arch_esf, r26);
GEN_OFFSET_STRUCT(arch_esf, r27);
GEN_OFFSET_STRUCT(arch_esf, r28);
GEN_OFFSET_STRUCT(arch_esf, r29_fp);
GEN_OFFSET_STRUCT(arch_esf, r30_sp);
GEN_OFFSET_STRUCT(arch_esf, r31_lr);
GEN_OFFSET_STRUCT(arch_esf, pc);
GEN_OFFSET_STRUCT(arch_esf, event_type);
GEN_OFFSET_STRUCT(arch_esf, event_info);

GEN_ABS_SYM_END
