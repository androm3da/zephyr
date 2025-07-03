/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_VM_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_VM_H_

/* Hexagon VM hypercall numbers */
#define HVM_TRAP0_VMVERSION     0
#define HVM_TRAP0_VMRTE         1
#define HVM_TRAP0_VMSETVEC      2
#define HVM_TRAP0_VMSETIE       3
#define HVM_TRAP0_VMGETIE       4
#define HVM_TRAP0_VMINTOP       5
#define HVM_TRAP0_VMCLRMAP      10
#define HVM_TRAP0_VMNEWMAP      11
#define HVM_TRAP0_VMCACHE       13
#define HVM_TRAP0_VMGETTIME     14
#define HVM_TRAP0_VMSETTIME     15
#define HVM_TRAP0_VMWAIT        16
#define HVM_TRAP0_VMYIELD       17
#define HVM_TRAP0_VMSTART       18
#define HVM_TRAP0_VMSTOP        19
#define HVM_TRAP0_VMVPID        20
#define HVM_TRAP0_VMSETREGS     21
#define HVM_TRAP0_VMGETREGS     22

/* VM interrupt operations */
#define VMINTOP_NOP             0
#define VMINTOP_ENABLE          1
#define VMINTOP_DISABLE         2
#define VMINTOP_CLEARALL        3
#define VMINTOP_CLEAR           4

#ifndef _ASMLANGUAGE

static inline long hvm_trap0(int nr)
{
	register long r0 asm("r0") = nr;

	asm volatile(
		"trap0(#0)"
		: "+r"(r0)
		:
		: "memory", "r1", "r2", "r3", "r4", "r5"
	);

	return r0;
}

static inline long hvm_trap0_1(int nr, long a1)
{
	register long r0 asm("r0") = nr;
	register long r1 asm("r1") = a1;

	asm volatile(
		"trap0(#0)"
		: "+r"(r0), "+r"(r1)
		:
		: "memory", "r2", "r3", "r4", "r5"
	);

	return r0;
}

static inline long hvm_trap0_2(int nr, long a1, long a2)
{
	register long r0 asm("r0") = nr;
	register long r1 asm("r1") = a1;
	register long r2 asm("r2") = a2;

	asm volatile(
		"trap0(#0)"
		: "+r"(r0), "+r"(r1), "+r"(r2)
		:
		: "memory", "r3", "r4", "r5"
	);

	return r0;
}

static inline long hvm_trap0_3(int nr, long a1, long a2, long a3)
{
	register long r0 asm("r0") = nr;
	register long r1 asm("r1") = a1;
	register long r2 asm("r2") = a2;
	register long r3 asm("r3") = a3;

	asm volatile(
		"trap0(#0)"
		: "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3)
		:
		: "memory", "r4", "r5"
	);

	return r0;
}

/* Helper functions for common VM operations */
static inline void hvm_set_vector(void *vector_base)
{
	hvm_trap0_1(HVM_TRAP0_VMSETVEC, (long)vector_base);
}

static inline void hvm_enable_interrupts(void)
{
	hvm_trap0_1(HVM_TRAP0_VMINTOP, VMINTOP_ENABLE);
}

static inline void hvm_disable_interrupts(void)
{
	hvm_trap0_1(HVM_TRAP0_VMINTOP, VMINTOP_DISABLE);
}

static inline uint64_t hvm_get_time(void)
{
	return hvm_trap0(HVM_TRAP0_VMGETTIME);
}

static inline void hvm_yield(void)
{
	hvm_trap0(HVM_TRAP0_VMYIELD);
}

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_VM_H_ */
