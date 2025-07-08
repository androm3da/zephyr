/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_

#ifndef _ASMLANGUAGE

#include <zephyr/kernel_structs.h>
#include <zephyr/types.h>
#include <zephyr/sys/util.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Architecture-specific kernel initialization */
static inline void arch_kernel_init(void)
{
	/* Initialize VM interface */
}

/* Thread context switching */
extern void arch_switch(void *switch_to, void **switched_from);

/* Thread creation */
extern void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack, char *stack_ptr,
			    k_thread_entry_t entry, void *p1, void *p2, void *p3);

/* Check if we're in an interrupt context */
static inline bool arch_is_in_isr(void)
{
	return false; /* TODO: Implement based on context */
}

/* Thread abortion */
extern void arch_thread_return_value_set(struct k_thread *thread, unsigned int value);

/* Stack pointer manipulation */
extern char *arch_k_thread_stack_buffer(k_thread_stack_t *stack);
extern size_t arch_k_thread_stack_size(k_thread_stack_t *stack);

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_ */
