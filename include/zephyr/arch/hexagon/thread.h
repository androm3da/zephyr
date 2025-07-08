/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

struct _callee_saved {
	/* Callee-saved registers (r16-r27) */
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

	/* Frame pointer and stack pointer */
	uint32_t r29_fp;
	uint32_t r30_sp;

	/* Link register */
	uint32_t r31_lr;
};

typedef struct _callee_saved _callee_saved_t;

struct _thread_arch {
	uint32_t swap_return_value;
	uint32_t vm_event_info[4]; /* g0-g3 for event handling */
};

typedef struct _thread_arch _thread_arch_t;

/* Stack frame for new threads */
struct init_stack_frame {
	/* Initial context - matches context switch expectations */
	struct hexagon_thread_context context;

	/* Space for thread entry parameters */
	uint32_t entry_point;
	uint32_t parameter1;
	uint32_t parameter2;
	uint32_t parameter3;
};

#endif /* _ASMLANGUAGE */

/* Stack alignment requirement */
#define ARCH_STACK_PTR_ALIGN 8

/* Thread stack size alignment */
#define ARCH_THREAD_STACK_SIZE_ALIGN 8

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_H_ */
