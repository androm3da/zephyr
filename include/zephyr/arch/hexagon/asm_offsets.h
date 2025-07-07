/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Assembly offsets for Hexagon HVM structures
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_ASM_OFFSETS_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_ASM_OFFSETS_H_

/* Thread info flags */
#define TIF_NEED_RESCHED    0
#define TIF_SIGPENDING      1
#define TIF_NOTIFY_RESUME   2
#define TIF_WORK_MASK       3
#define TIF_ALLWORK_MASK    7

/* Thread info structure offsets */
#define _THREAD_INFO_FLAGS  0

#define HVM_VMEST_CAUSE_TRAP       0x01
#define HVM_VMEST_CAUSE_TRAP0      0x02
#define HVM_VMEST_CAUSE_MACHCHECK  0x03
#define HVM_VMEST_CAUSE_GENEX      0x04
#define HVM_VMEST_CAUSE_PMUREAD    0x05
#define HVM_VMEST_CAUSE_INTERRUPT  0x06

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_ASM_OFFSETS_H_ */
