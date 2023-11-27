/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Hexagon specific syscall header
 *
 * This header contains the Hexagon specific syscall interface.  It is
 * included by the syscall interface architecture-abstraction header
 * (include/arch/syscall.h)
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_SYSCALL_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_SYSCALL_H_

/*
 * Privileged mode system calls
 */
#define RV_ECALL_RUNTIME_EXCEPT 0
#define RV_ECALL_IRQ_OFFLOAD    1

#ifndef _ASMLANGUAGE

#include <zephyr/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define hexagon_syscall(...)                                                                       \
	do {                                                                                       \
		__asm__ __volatile__("trap0(#1)" : "=r"(r0) : __VA_ARGS__ : "memory");             \
		return r0;                                                                         \
	} while (0)

/*
 * Syscall invocation macros. hexagon-specific machine constraints used to ensure
 * args land in the proper registers.
 */
static inline uintptr_t arch_syscall_invoke6(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
					     uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
					     uintptr_t call_id)
{
	register uint32_t r6 __asm__("r6") = call_id;
	register uint32_t r0 __asm__("r0") = arg1;
	register uint32_t r1 __asm__("r1") = arg2;
	register uint32_t r2 __asm__("r2") = arg3;
	register uint32_t r3 __asm__("r3") = arg4;
	register uint32_t r4 __asm__("r4") = arg5;
	register uint32_t r5 __asm__("r5") = arg6;

	hexagon_syscall("r"(r6), "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5));
}

static inline uintptr_t arch_syscall_invoke5(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
					     uintptr_t arg4, uintptr_t arg5, uintptr_t call_id)
{
	register uint32_t r6 __asm__("r6") = call_id;
	register uint32_t r0 __asm__("r0") = arg1;
	register uint32_t r1 __asm__("r1") = arg2;
	register uint32_t r2 __asm__("r2") = arg3;
	register uint32_t r3 __asm__("r3") = arg4;
	register uint32_t r4 __asm__("r4") = arg5;

	hexagon_syscall("r"(r6), "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4));
}

static inline uintptr_t arch_syscall_invoke4(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
					     uintptr_t arg4, uintptr_t call_id)
{
	register uint32_t r6 __asm__("r6") = call_id;
	register uint32_t r0 __asm__("r0") = arg1;
	register uint32_t r1 __asm__("r1") = arg2;
	register uint32_t r2 __asm__("r2") = arg3;
	register uint32_t r3 __asm__("r3") = arg4;

	hexagon_syscall("r"(r6), "0"(r0), "r"(r1), "r"(r2), "r"(r3));
}

static inline uintptr_t arch_syscall_invoke3(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
					     uintptr_t call_id)
{
	register uint32_t r6 __asm__("r6") = call_id;
	register uint32_t r0 __asm__("r0") = arg1;
	register uint32_t r1 __asm__("r1") = arg2;
	register uint32_t r2 __asm__("r2") = arg3;

	hexagon_syscall("r"(r6), "0"(r0), "r"(r1), "r"(r2));
}

static inline uintptr_t arch_syscall_invoke2(uintptr_t arg1, uintptr_t arg2, uintptr_t call_id)
{
	register uint32_t r6 __asm__("r6") = call_id;
	register uint32_t r0 __asm__("r0") = arg1;
	register uint32_t r1 __asm__("r1") = arg2;

	hexagon_syscall("r"(r6), "0"(r0), "r"(r1));
}

static inline uintptr_t arch_syscall_invoke1(uintptr_t arg1, uintptr_t call_id)
{
	register uint32_t r6 __asm__("r6") = call_id;
	register uint32_t r0 __asm__("r0") = arg1;

	hexagon_syscall("r"(r6), "0"(r0));
}

static inline uintptr_t arch_syscall_invoke0(uintptr_t call_id)
{
	register uint32_t r6 __asm__("r6") = call_id;
	register uint32_t r0 __asm__("r0");

	hexagon_syscall("r"(r6));
}

#ifdef CONFIG_USERSPACE
static inline bool arch_is_user_context(void)
{
	static const uint32_t UM = 1 << 16;
	static const uint32_t EX = 1 << 17;
	static const uint32_t GM = 1 << 19;
	uint32_t ssr = read_ssr();
	bool is_user_mode = (ssr & (UM | GM | EX)) == UM;
	return is_user_mode;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */
#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_SYSCALL_H_ */
