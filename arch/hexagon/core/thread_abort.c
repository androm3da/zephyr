/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon k_thread_abort() routine
 */

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>
#include <ksched.h>
#include <kswap.h>

void z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	/* For Hexagon, we can use the generic thread abort implementation */
	z_thread_abort(thread);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
