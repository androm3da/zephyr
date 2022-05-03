/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * based on include/arch/mips/thread.h
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Per-arch thread definition
 *
 * This file contains definitions for
 *
 *  struct _thread_arch
 *  struct _callee_saved
 *
 * necessary to instantiate instances of struct k_thread.
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_THREAD_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

/*
 * The following structure defines the list of registers that need to be
 * saved/restored when a cooperative context switch occurs.
 */
struct _callee_saved {
	ulong_t r16; /* saved register: scratch */
	ulong_t r17; /* saved register: scratch */
	ulong_t r18; /* saved register: scratch */
	ulong_t r19; /* saved register: scratch */
	ulong_t r20; /* saved register: scratch */
	ulong_t r21; /* saved register: scratch */
	ulong_t r22; /* saved register: scratch */
	ulong_t r23; /* saved register: scratch */
	ulong_t r24; /* saved register: scratch */
	ulong_t r25; /* saved register: scratch */
	ulong_t r26; /* saved register: scratch */
	ulong_t r27; /* saved register: scratch */

	ulong_t r29; /* saved register: call frame */
	ulong_t r30; /* saved register: call frame */
	ulong_t r31; /* saved register: call frame */
};
typedef struct _callee_saved _callee_saved_t;

struct _thread_arch {
	/* FIXME */
	uint32_t swap_return_value;
	int prio;
};

typedef struct _thread_arch _thread_arch_t;

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_THREAD_H_ */
