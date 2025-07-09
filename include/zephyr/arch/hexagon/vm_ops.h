/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_VM_OPS_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_VM_OPS_H_

/* VM operation codes */
#define HEXAGON_VM_vmversion 0x0
#define HEXAGON_VM_vmrte     0x1
#define HEXAGON_VM_vmsetvec  0x2
#define HEXAGON_VM_vmsetie   0x3
#define HEXAGON_VM_vmgetie   0x4
#define HEXAGON_VM_vmintop   0x5
#define HEXAGON_VM_vmclrmap  0xa
#define HEXAGON_VM_vmnewmap  0xb
#define HEXAGON_VM_vmcache   0xd
#define HEXAGON_VM_vmgettime 0xe
#define HEXAGON_VM_vmsettime 0xf
#define HEXAGON_VM_vmwait    0x10
#define HEXAGON_VM_vmyield   0x11
#define HEXAGON_VM_vmstart   0x12
#define HEXAGON_VM_vmstop    0x13
#define HEXAGON_VM_vmvpid    0x14
#define HEXAGON_VM_vmsetregs 0x15
#define HEXAGON_VM_vmgetregs 0x16
#define HEXAGON_VM_vmtimerop 0x18
#define HEXAGON_VM_vmgetinfo 0x1a

#ifndef _ASMLANGUAGE

#include <zephyr/types.h>

static inline uint32_t hexagon_vm_vmversion(uint32_t version)
{
	register uint32_t r0 __asm__("r0") = version;
	__asm__ volatile("trap0(#0x0)" : "+r"(r0) : : "memory", "p0", "p1", "p2", "p3");
	return r0;
}

static inline uint32_t hexagon_vm_vmsetvec(void *vector_base)
{
	register uint32_t r0 __asm__("r0") = (uint32_t)vector_base;
	__asm__ volatile("trap0(#0x2)" : "+r"(r0) : : "memory", "p0", "p1", "p2", "p3");
	return r0;
}

static inline uint32_t hexagon_vm_vmsetie(uint32_t enable)
{
	register uint32_t r0 __asm__("r0") = enable;
	__asm__ volatile("trap0(#0x3)" : "+r"(r0) : : "memory", "p0", "p1", "p2", "p3");
	return r0;
}

static inline uint32_t hexagon_vm_vmgetie(void)
{
	register uint32_t r0 __asm__("r0");
	__asm__ volatile("trap0(#0x4)" : "=r"(r0) : : "memory", "p0", "p1", "p2", "p3");
	return r0;
}

static inline void hexagon_vm_vmrte(void)
{
	__asm__ volatile("trap0(#0x1)" : : : "memory");
}

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_VM_OPS_H_ */
