/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

/* Legacy callee_saved structure required by Zephyr core */
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

/* Thread flags */
#define HEXAGON_THREAD_FLAG_ABORT   0x01
#define HEXAGON_THREAD_FLAG_FP_USED 0x02

/* Hexagon thread context structure */
struct hexagon_thread_context {
	/* Callee-saved registers */
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
	uint32_t r28; /* Frame pointer */
	uint32_t r29; /* Stack pointer */
	uint32_t r30; /* Link register */
	uint32_t r31; /* Another FP/AP */

	/* Control registers */
	uint32_t usr; /* User status register */
	uint32_t ugp; /* User global pointer */

	/* Loop registers (if in use) */
	uint32_t lc0;
	uint32_t lc1;
	uint32_t sa0;
	uint32_t sa1;

	/* Predicate registers (callee-saved portion) */
	uint8_t p3_0; /* P3:0 packed */

	/* VM guest registers for event handling */
	uint32_t g0;
	uint32_t g1;
	uint32_t g2;
	uint32_t g3;
};

/* Extended thread structure */
struct _thread_arch {
	/* Context for thread switching */
	struct hexagon_thread_context context;

	/* Return value from arch_switch */
	uint32_t swap_return_value;

	/* Thread privilege level */
	uint8_t priv_level;

	/* Flags */
	uint8_t flags;

	/* VM event info for compatibility with existing code */
	uint32_t vm_event_info[4];

	/* Hardware thread ID (-1 if not a hardware thread) */
	int8_t hw_thread_id;

	/* Thread-local storage pointer */
	void *tls_ptr;

#ifdef CONFIG_HW_STACK_PROTECTION
	/* Stack protection FRAMELIMIT value */
	uint32_t framelimit;
#endif
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
