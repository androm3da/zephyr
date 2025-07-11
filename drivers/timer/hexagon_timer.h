/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_DRIVERS_TIMER_HEXAGON_TIMER_H_
#define ZEPHYR_DRIVERS_TIMER_HEXAGON_TIMER_H_

#include <zephyr/types.h>

/* Timer operation codes for vmtimerop */
#define HEXAGON_TIMER_GETFREQ     0
#define HEXAGON_TIMER_GETRES      1
#define HEXAGON_TIMER_GETTIME     2
#define HEXAGON_TIMER_GETTIMEOUT  3
#define HEXAGON_TIMER_SETTIMEOUT  4
#define HEXAGON_TIMER_DELTATIMEOUT 5

/* Timer IRQ number - typically 0 for system timer */
#define HEXAGON_TIMER_IRQ 0
#define HEXAGON_TIMER_IRQ_PRIORITY 0

/* Timer configuration */
struct hexagon_timer_config {
	uint32_t freq;
	uint32_t irq;
	uint32_t priority;
};

/* Timer runtime data */
struct hexagon_timer_data {
	uint32_t accumulated_cycles;
	uint32_t last_count;
	uint32_t cycles_per_tick;
};

#endif /* ZEPHYR_DRIVERS_TIMER_HEXAGON_TIMER_H_ */