/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_

#include <kernel_arch_data.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ASMLANGUAGE

static ALWAYS_INLINE void arch_kernel_init(void)
{
    /* Hexagon-specific initialization */
}

/* arch_thread_return_value_set is provided by kernel_internal.h when CONFIG_USE_SWITCH is enabled */

extern void z_hexagon_setup_thread_stack(struct k_thread *thread,
                                        k_thread_stack_t *stack,
                                        size_t stack_size,
                                        k_thread_entry_t entry,
                                        void *p1, void *p2, void *p3);

static inline void arch_switch_to_main_thread(struct k_thread *main_thread,
                                             k_thread_stack_t *main_stack,
                                             size_t main_stack_size,
                                             k_thread_entry_t _main)
{
    /* Set up main thread context */
    z_hexagon_setup_thread_stack(main_thread, main_stack, main_stack_size,
                                _main, NULL, NULL, NULL);
}

extern void z_hexagon_context_switch(struct k_thread *new, void **old_switch_handle);

#define arch_switch(new, old) z_hexagon_context_switch(new, old)

#endif /* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_KERNEL_ARCH_FUNC_H_ */
