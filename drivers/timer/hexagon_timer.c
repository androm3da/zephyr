/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys_clock.h>
#include <zephyr/spinlock.h>
#include <zephyr/irq.h>
#include "hexagon_timer.h"

static struct hexagon_timer_data timer_data;
static struct k_spinlock timer_lock;
static bool timer_initialized = false;

/* Execute timer operation */
static inline uint64_t hexagon_timer_op(uint32_t op, uint64_t arg)
{
	register uint32_t r0 __asm__("r0") = op;
	register uint32_t r1 __asm__("r1") = (uint32_t)(arg & 0xFFFFFFFF);
	register uint32_t r2 __asm__("r2") = (uint32_t)(arg >> 32);
	register uint32_t r3 __asm__("r3");

	__asm__ volatile("trap1(#0x18)" /* vmtimerop */
			 : "+r"(r0), "=r"(r3)
			 : "r"(r1), "r"(r2)
			 : "memory");

	return ((uint64_t)r3 << 32) | r0;
}

/* Simple cycle counter access using Hexagon hardware registers */
static inline uint32_t hexagon_get_cycles_32(void)
{
	uint32_t cycles;
	__asm__ volatile("r0 = pcyclelo" : "=r"(cycles) : : "memory");
	return cycles;
}

static inline uint64_t hexagon_get_cycles_64(void)
{
	uint32_t lo, hi;
	__asm__ volatile("r0 = pcyclelo\n\t"
			 "r1 = pcyclehi"
			 : "=r"(lo), "=r"(hi)
			 :
			 : "memory");
	return ((uint64_t)hi << 32) | lo;
}

/* Get timer frequency */
static uint32_t hexagon_timer_get_freq(void)
{
	return (uint32_t)hexagon_timer_op(HEXAGON_TIMER_GETFREQ, 0);
}

/* Get timer resolution */
static uint32_t hexagon_timer_get_res(void)
{
	return (uint32_t)hexagon_timer_op(HEXAGON_TIMER_GETRES, 0);
}

/* Get current timer value */
static uint64_t hexagon_timer_get_time(void)
{
	return hexagon_timer_op(HEXAGON_TIMER_GETTIME, 0);
}

/* Set timeout for next interrupt */
static void hexagon_timer_set_timeout(uint64_t timeout)
{
	hexagon_timer_op(HEXAGON_TIMER_SETTIMEOUT, timeout);
}

/* Set delta timeout */
static void hexagon_timer_set_delta_timeout(uint32_t delta)
{
	hexagon_timer_op(HEXAGON_TIMER_DELTATIMEOUT, delta);
}

/* Timer interrupt handler */
static void hexagon_timer_isr(const void *arg)
{
	ARG_UNUSED(arg);
	k_spinlock_key_t key = k_spin_lock(&timer_lock);

	/* Accumulate cycles */
	timer_data.accumulated_cycles += timer_data.cycles_per_tick;

	/* Set next timeout */
	hexagon_timer_set_delta_timeout(timer_data.cycles_per_tick);

	k_spin_unlock(&timer_lock, key);

	/* Announce timer tick to kernel */
	sys_clock_announce(1);
}

/* Initialize system timer - called by kernel during early init */
void sys_clock_driver_init(void)
{
	/* Use default frequency - avoid VM operations during early init */
	uint32_t freq = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

	/* Calculate cycles per tick */
	timer_data.cycles_per_tick = freq / CONFIG_SYS_CLOCK_TICKS_PER_SEC;
	timer_data.accumulated_cycles = 0;
	timer_data.last_count = 0;

	/* For now, don't set up interrupts to avoid semihosting issues */
#if 0
	/* TODO: Enable when timer initialization sequence is resolved */
	arch_irq_connect_dynamic(HEXAGON_TIMER_IRQ, HEXAGON_TIMER_IRQ_PRIORITY,
				 hexagon_timer_isr, NULL, 0);
	hexagon_timer_set_delta_timeout(timer_data.cycles_per_tick);
	irq_enable(HEXAGON_TIMER_IRQ);
#endif

	timer_initialized = true;
}

/* Get elapsed cycles */
uint32_t sys_clock_cycle_get_32(void)
{
#if __has_builtin(__builtin_readcyclecounter)
	return __builtin_readcyclecounter();
#else
#error no cycle counter builtin
#endif
}

/* Get elapsed cycles (64-bit) */
uint64_t sys_clock_cycle_get_64(void)
{
#if __has_builtin(__builtin_readcyclecounter)
	return __builtin_readcyclecounter();
#else
#error no cycle counter builtin
#endif
}

/* Set alarm for absolute time */
void sys_clock_set_timeout(int32_t ticks, bool idle)
{
	ARG_UNUSED(idle);

	if (!timer_initialized) {
		return;
	}

	if (ticks == K_TICKS_FOREVER) {
		/* Disable timer */
		irq_disable(HEXAGON_TIMER_IRQ);
		return;
	}

	if (ticks <= 0) {
		ticks = 1;
	}

	k_spinlock_key_t key = k_spin_lock(&timer_lock);

	/* Calculate timeout in cycles */
	uint32_t timeout_cycles = ticks * timer_data.cycles_per_tick;

	/* Set new timeout */
	hexagon_timer_set_delta_timeout(timeout_cycles);

	/* Re-enable timer if it was disabled */
	irq_enable(HEXAGON_TIMER_IRQ);

	k_spin_unlock(&timer_lock, key);
}

/* Get remaining ticks until next timeout */
uint32_t sys_clock_elapsed(void)
{
	if (!timer_initialized) {
		return 0;
	}

	k_spinlock_key_t key = k_spin_lock(&timer_lock);
	uint32_t elapsed = timer_data.accumulated_cycles / timer_data.cycles_per_tick;
	k_spin_unlock(&timer_lock, key);

	return elapsed;
}

/* Disable system timer */
void sys_clock_disable(void)
{
	if (timer_initialized) {
		irq_disable(HEXAGON_TIMER_IRQ);
	}
}

static inline void _pause(void) {
	__asm__ volatile("pause(#255)" ::: );
}

/* Busy wait implementation */
void arch_busy_wait(uint32_t usec_to_wait)
{
	if (timer_initialized) {
		uint64_t start = hexagon_get_cycles_64();
		uint32_t freq = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;
		uint64_t cycles_to_wait = ((uint64_t)usec_to_wait * freq) / 1000000ULL;

		while ((hexagon_get_cycles_64() - start) < cycles_to_wait) {
			_pause();
		}
	} else {
		/* Fallback to simple nop loop if timer not available */
		volatile uint32_t delay_loops = usec_to_wait * 10;
		for (volatile uint32_t i = 0; i < delay_loops; i++) {
			_pause();
		}
	}
}

static int hexagon_timer_init(void)
{
	/* Timer initialization moved to sys_clock_driver_init() */
	/* TODO: Enable timer interrupt setup when initialization sequence is resolved */
	return 0;
}

/* SYS_INIT breaks semihosting - commented out for now */
/* TODO: Find alternative initialization approach */
/* SYS_INIT(hexagon_timer_init, PRE_KERNEL_2, CONFIG_SYSTEM_CLOCK_INIT_PRIORITY); */
