/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SOC_H_
#define __SOC_H_

#include <zephyr/sys/util.h>

#ifndef _ASMLANGUAGE

/* Timer */
#define HEXAGON_TIMER_FREQ CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC

/* Cache line size */
#define HEXAGON_CACHE_LINE_SIZE 32

/* Hexagon-specific CPU features */
#if defined(CONFIG_SOC_HEXAGON_V66)
#define HEXAGON_HAS_L2_CACHE
#define HEXAGON_HAS_HVX
#define HEXAGON_HAS_AUDIO_EXT
#endif

/* Memory map */
#define HEXAGON_SRAM_BASE   CONFIG_HEXAGON_SRAM_BASE_ADDRESS
#define HEXAGON_SRAM_SIZE   (CONFIG_HEXAGON_SRAM_SIZE * 1024)
#define HEXAGON_FLASH_BASE  CONFIG_HEXAGON_FLASH_BASE_ADDRESS
#define HEXAGON_FLASH_SIZE  (CONFIG_HEXAGON_FLASH_SIZE * 1024)

/* Peripheral base addresses */
#define HEXAGON_UART0_BASE  0x1c090000
#define HEXAGON_TIMER_BASE  0x1c0a0000
#define HEXAGON_INTC_BASE   0x1c0b0000

extern void soc_early_init(void);

#endif /* !_ASMLANGUAGE */

#endif /* __SOC_H_ */
