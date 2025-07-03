/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>

extern void z_thread_entry_wrapper(k_thread_entry_t entry,
                                  void *p1, void *p2, void *p3);

void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack,
                    size_t stackSize, k_thread_entry_t entry,
                    void *p1, void *p2, void *p3,
                    int prio, unsigned int options)
{
    char *pStackMem = Z_THREAD_STACK_BUFFER(stack);
    void *stackEnd;

    /* Hexagon stack grows down */
    stackEnd = pStackMem + stackSize;

    /* Align stack */
    stackEnd = (void *)ROUND_DOWN((uintptr_t)stackEnd, STACK_ALIGN);

    /* Initialize thread structure */
    thread->arch.stack_top = stackEnd;
    thread->arch.stack_size = stackSize;

    /* Set up initial context */
    thread->arch.callee_saved.sp = (uint32_t)stackEnd;
    thread->arch.callee_saved.fp = 0;
    thread->arch.callee_saved.lr = (uint32_t)z_thread_entry_wrapper;

    /* Entry parameters in r16-r19 */
    thread->arch.callee_saved.r16 = (uint32_t)entry;
    thread->arch.callee_saved.r17 = (uint32_t)p1;
    thread->arch.callee_saved.r18 = (uint32_t)p2;
    thread->arch.callee_saved.r19 = (uint32_t)p3;

    /* Initialize predicate registers */
    thread->arch.callee_saved.p3_0 = 0;

    /* Initialize user status register */
    thread->arch.callee_saved.usr = 0;

    thread->arch.swap_return_value = -EAGAIN;
}

void z_hexagon_setup_thread_stack(struct k_thread *thread,
                                 k_thread_stack_t *stack,
                                 size_t stack_size,
                                 k_thread_entry_t entry,
                                 void *p1, void *p2, void *p3)
{
    arch_new_thread(thread, stack, stack_size, entry, p1, p2, p3, 0, 0);
}
