/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <toolchain.h>
#include <linker/sections.h>
#include <offsets_short.h>
#include <arch/cpu.h>
#include <sys/util.h>
#include <kernel.h>
//#include <arch/hexagon/syscall.h>

GTEXT(z_hexagon_thread_start)

#define SP r29


#if defined(CONFIG_HVX)
#define DO_CALLER_SAVED_HVX() \
	v0 = memw(SP+ __z_arch_esf_t_v0_OFFSET) \
	/* TODO */

#endif

z_hexagon_thread_start:
1:	pause(#255)
	jump 1b
