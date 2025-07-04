/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>
#include <zephyr/spinlock.h>

/* Simple timer implementation for Hexagon */

static uint64_t cycle_count = 0;
static struct k_spinlock timer_lock;

/* Convert cycles to nanoseconds */
static inline uint64_t cyc_to_ns_floor64(uint64_t cyc)
{
    /* Simple conversion assuming 500MHz clock */
    return (cyc * 1000ULL) / 500ULL;
}

uint32_t sys_clock_elapsed(void)
{
    static uint64_t last_cycle = 0;
    uint64_t now;
    uint32_t elapsed;
    k_spinlock_key_t key;
    
    key = k_spin_lock(&timer_lock);
    now = ++cycle_count;
    elapsed = (uint32_t)(now - last_cycle);
    last_cycle = now;
    k_spin_unlock(&timer_lock, key);
    
    return elapsed;
}

/* sys_clock_tick_get is provided by kernel/timeout.c */

uint32_t sys_clock_cycle_get_32(void)
{
    return (uint32_t)++cycle_count;
}

void sys_clock_set_timeout(int32_t ticks, bool idle)
{
    ARG_UNUSED(ticks);
    ARG_UNUSED(idle);
    /* Timer timeout stub */
}

void sys_clock_idle_exit(void)
{
    /* Timer idle exit stub */
}