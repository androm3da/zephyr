/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_

#ifndef _ASMLANGUAGE

#include <zephyr/kernel_structs.h>
#include <zephyr/types.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Thread context switching */
extern void z_hexagon_arch_switch(void *switch_to, void **switched_from);
static ALWAYS_INLINE void arch_switch(void *switch_to, void **switched_from)
{
	z_hexagon_arch_switch(switch_to, switched_from);
}

/*
 * CONFIG_HW_STACK_PROTECTION wiring:
 *
 * z_arch_stack_protection_setup() is called from arch_new_thread() in
 * arch/hexagon/core/thread.c whenever a new thread is created.
 *
 * z_arch_stack_protection_switch() must be called on every context switch
 * to update the FRAMELIMIT register for the incoming thread.  It is invoked
 * from z_hexagon_do_stack_switch() in arch/hexagon/core/stack_protect.c,
 * which in turn is called from the EVENT_EXIT preemption path via
 * z_hexagon_arch_switch().  (Wiring is arch-internal; callers outside
 * arch/hexagon/ should use the generic CONFIG_HW_STACK_PROTECTION Kconfig.)
 */

/* Thread creation */
extern void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack, char *stack_ptr,
			    k_thread_entry_t entry, void *p1, void *p2, void *p3);

/* Thread abortion */
/* arch_thread_return_value_set is only needed for !CONFIG_USE_SWITCH */

/* Stack pointer manipulation */
extern char *arch_k_thread_stack_buffer(k_thread_stack_t *stack);

/* Stack protection functions */
#ifdef CONFIG_HW_STACK_PROTECTION
extern void z_arch_stack_protection_setup(struct k_thread *thread);
extern void z_arch_stack_protection_switch(struct k_thread *old_thread,
					   struct k_thread *new_thread);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_ */
