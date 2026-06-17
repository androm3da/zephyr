/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/debug/coredump.h>
#include <zephyr/arch/hexagon/exception.h>
#include <hvx.h>

#define ARCH_HDR_VER 1

struct hexagon_arch_block {
	struct {
		/* General-purpose registers r0-r31 */
		uint32_t r0;
		uint32_t r1;
		uint32_t r2;
		uint32_t r3;
		uint32_t r4;
		uint32_t r5;
		uint32_t r6;
		uint32_t r7;
		uint32_t r8;
		uint32_t r9;
		uint32_t r10;
		uint32_t r11;
		uint32_t r12;
		uint32_t r13;
		uint32_t r14;
		uint32_t r15;
		uint32_t r16;
		uint32_t r17;
		uint32_t r18;
		uint32_t r19;
		uint32_t r20;
		uint32_t r21;
		uint32_t r22;
		uint32_t r23;
		uint32_t r24;
		uint32_t r25;
		uint32_t r26;
		uint32_t r27;
		uint32_t r28;
		uint32_t r29_sp; /* Stack pointer */
		uint32_t r30_fp; /* Frame pointer */
		uint32_t r31_lr; /* Link register */

		/* Control registers */
		uint32_t pc;         /* Program counter */
		uint32_t event_type; /* Exception/event type */

		/* Guest registers (g0-g3) containing exception info */
		uint32_t g0;
		uint32_t g1;
		uint32_t g2;
		uint32_t g3;
	} r;
} __packed;

/*
 * This might be too large for stack space if defined
 * inside function. So do it here.
 */
static struct hexagon_arch_block arch_blk;

#ifdef CONFIG_HEXAGON_HVX
void arch_coredump_hvx_dump(void)
{
	struct hvx_context *hvx_ctx;
	struct coredump_arch_hdr_t hvx_hdr;

	if (!hvx_is_available()) {
		return;
	}

	hvx_ctx = hvx_get_current_context();
	if (hvx_ctx == NULL || !hvx_ctx->context_valid) {
		return;
	}

	if (hvx_ctx->context_dirty) {
		hvx_context_save();
	}

	hvx_hdr.id = 'H';
	hvx_hdr.hdr_version = 1;
	hvx_hdr.num_bytes = sizeof(struct hvx_vectors);

	coredump_buffer_output((uint8_t *)&hvx_hdr, sizeof(hvx_hdr));
	coredump_buffer_output((uint8_t *)hvx_ctx->vregs, sizeof(struct hvx_vectors));

	struct {
		uint32_t context_num;
		uint32_t generation;
		uint32_t vector_size;
		uint32_t context_valid;
		uint32_t context_dirty;
	} hvx_info = {
		.context_num = hvx_ctx->context_num,
		.generation = hvx_ctx->generation,
		.vector_size = HVX_VECTOR_SIZE,
		.context_valid = hvx_ctx->context_valid,
		.context_dirty = hvx_ctx->context_dirty,
	};

	struct coredump_arch_hdr_t hvx_info_hdr = {
		.id = 'I',
		.hdr_version = 1,
		.num_bytes = sizeof(hvx_info),
	};

	coredump_buffer_output((uint8_t *)&hvx_info_hdr, sizeof(hvx_info_hdr));
	coredump_buffer_output((uint8_t *)&hvx_info, sizeof(hvx_info));
}
#endif /* CONFIG_HEXAGON_HVX */

void arch_coredump_info_dump(const struct arch_esf *esf)
{
	struct coredump_arch_hdr_t hdr = {
		.id = COREDUMP_ARCH_HDR_ID,
		.hdr_version = ARCH_HDR_VER,
		.num_bytes = sizeof(arch_blk),
	};

	if (esf == NULL) {
		return;
	}

	(void)memset(&arch_blk, 0, sizeof(arch_blk));

	arch_blk.r.r0 = esf->r0;
	arch_blk.r.r1 = esf->r1;
	arch_blk.r.r2 = esf->r2;
	arch_blk.r.r3 = esf->r3;
	arch_blk.r.r4 = esf->r4;
	arch_blk.r.r5 = esf->r5;
	arch_blk.r.r6 = esf->r6;
	arch_blk.r.r7 = esf->r7;
	arch_blk.r.r8 = esf->r8;
	arch_blk.r.r9 = esf->r9;
	arch_blk.r.r10 = esf->r10;
	arch_blk.r.r11 = esf->r11;
	arch_blk.r.r12 = esf->r12;
	arch_blk.r.r13 = esf->r13;
	arch_blk.r.r14 = esf->r14;
	arch_blk.r.r15 = esf->r15;
	arch_blk.r.r16 = esf->r16;
	arch_blk.r.r17 = esf->r17;
	arch_blk.r.r18 = esf->r18;
	arch_blk.r.r19 = esf->r19;
	arch_blk.r.r20 = esf->r20;
	arch_blk.r.r21 = esf->r21;
	arch_blk.r.r22 = esf->r22;
	arch_blk.r.r23 = esf->r23;
	arch_blk.r.r24 = esf->r24;
	arch_blk.r.r25 = esf->r25;
	arch_blk.r.r26 = esf->r26;
	arch_blk.r.r27 = esf->r27;
	arch_blk.r.r28 = esf->r28;
	arch_blk.r.r29_sp = esf->r29_sp;
	arch_blk.r.r30_fp = esf->r30_fp;
	arch_blk.r.r31_lr = esf->r31_lr;

	arch_blk.r.pc = esf->pc;
	arch_blk.r.event_type = esf->event_type;
	arch_blk.r.g0 = esf->event_info[0];
	arch_blk.r.g1 = esf->event_info[1];
	arch_blk.r.g2 = esf->event_info[2];
	arch_blk.r.g3 = esf->event_info[3];

	coredump_buffer_output((uint8_t *)&hdr, sizeof(hdr));
	coredump_buffer_output((uint8_t *)&arch_blk, sizeof(arch_blk));

#ifdef CONFIG_HEXAGON_HVX
	arch_coredump_hvx_dump();
#endif
}

uint16_t arch_coredump_tgt_code_get(void)
{
	return COREDUMP_TGT_HEXAGON;
}

#ifdef CONFIG_DEBUG_COREDUMP_THREAD_STACK_TOP
uintptr_t arch_coredump_stack_ptr_get(const struct k_thread *thread)
{
	return (uintptr_t)thread->switch_handle;
}
#endif /* CONFIG_DEBUG_COREDUMP_THREAD_STACK_TOP */

#ifdef CONFIG_DEBUG_COREDUMP_DUMP_THREAD_PRIV_STACK
void arch_coredump_priv_stack_dump(struct k_thread *thread)
{
	/*
	 * Not implemented: Hexagon does not yet have a separate privileged
	 * stack for user-mode threads.  When CONFIG_USERSPACE support is
	 * completed and a priv stack is added, this function should dump it
	 * via coredump_buffer_output().
	 */
	ARG_UNUSED(thread);
}
#endif /* CONFIG_DEBUG_COREDUMP_DUMP_THREAD_PRIV_STACK */
