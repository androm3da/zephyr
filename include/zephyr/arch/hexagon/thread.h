/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Per-arch thread definition
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_THREAD_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

/* Forward declaration to avoid including hvx.h from thread.h */
struct hvx_context;

/**
 * @brief Callee-saved register context for cooperative context switching.
 */
struct _callee_saved {
	/** General-purpose register R16 (callee-saved). */
	uint32_t r16;
	/** General-purpose register R17 (callee-saved). */
	uint32_t r17;
	/** General-purpose register R18 (callee-saved). */
	uint32_t r18;
	/** General-purpose register R19 (callee-saved). */
	uint32_t r19;
	/** General-purpose register R20 (callee-saved). */
	uint32_t r20;
	/** General-purpose register R21 (callee-saved). */
	uint32_t r21;
	/** General-purpose register R22 (callee-saved). */
	uint32_t r22;
	/** General-purpose register R23 (callee-saved). */
	uint32_t r23;
	/** General-purpose register R24 (callee-saved). */
	uint32_t r24;
	/** General-purpose register R25 (callee-saved). */
	uint32_t r25;
	/** General-purpose register R26 (callee-saved). */
	uint32_t r26;
	/** General-purpose register R27 (callee-saved). */
	uint32_t r27;

	/** Stack pointer (R29). */
	uint32_t r29_sp;
	/** Frame pointer (R30). */
	uint32_t r30_fp;

	/** Link register (R31). */
	uint32_t r31_lr;
};

typedef struct _callee_saved _callee_saved_t;

/* Thread flags */
#define HEXAGON_THREAD_FLAG_ABORT      0x01
#define HEXAGON_THREAD_FLAG_FP_USED    0x02
#define HEXAGON_THREAD_FLAG_STACK_PROT 0x04

/**
 * @brief Architecture-specific thread data.
 */
struct _thread_arch {
	/** Return value from arch_switch. */
	uint32_t swap_return_value;

	/* Thread privilege level */
	uint8_t priv_level;

	/* Flags */
	uint8_t flags;

	/* Hardware thread ID (-1 if not a hardware thread) */
	int8_t hw_thread_id;

	/* Thread-local storage pointer */
	void *tls_ptr;

	/* User global pointer (UGP) for TLS */
	uint32_t ugp;

#ifdef CONFIG_HW_STACK_PROTECTION
	/* Stack protection FRAMELIMIT value */
	uint32_t framelimit;
#endif

#ifdef CONFIG_USERSPACE
	/* Original entry point and arguments for K_USER threads */
	void (*user_entry)(void *, void *, void *);
	void *user_p1;
	void *user_p2;
	void *user_p3;
#endif

#ifdef CONFIG_HEXAGON_HVX
	/*
	 * Per-thread HVX context pointer.  Using a dedicated field here
	 * (rather than k_thread_custom_data) avoids conflicting with
	 * application use of the custom-data slot.
	 *
	 * NULL means this thread has not allocated an HVX context.
	 */
	struct hvx_context *hvx_ctx;
#endif
};

typedef struct _thread_arch _thread_arch_t;

#endif /* _ASMLANGUAGE */

/* Hexagon requires 8-byte stack alignment. */
#define ARCH_STACK_PTR_ALIGN 8

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_THREAD_H_ */
