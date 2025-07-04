/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

/* Simple stub context switch function */
void z_hexagon_context_switch(struct k_thread *new, void **old_switch_handle)
{
	/* For now, this is a stub - actual context switching would happen here */
}

/* Fatal error handler */
void z_hexagon_fatal_error(unsigned int reason, const struct arch_esf *esf)
{
	/* Simple fatal error handling - just loop forever */
	for (;;) {
		/* Hang */
	}
}