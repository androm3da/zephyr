/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_H_

#include <zephyr/devicetree.h>
#include <zephyr/arch/hexagon/thread.h>
#include <zephyr/arch/hexagon/exception.h>
#include <zephyr/arch/common/sys_bitops.h>
#include <zephyr/arch/common/ffs.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel/mm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hexagon VM virtual instructions */
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

/* Event types */
#define HEXAGON_EVENT_RESERVED1         0
#define HEXAGON_EVENT_MACHINE_CHECK     1
#define HEXAGON_EVENT_GENERAL_EXCEPTION 2
#define HEXAGON_EVENT_TRAP0             5
#define HEXAGON_EVENT_INTERRUPT         7

/* Stack alignment requirement */
#define ARCH_STACK_PTR_ALIGN 8

/* Architecture-specific kernel initialization */
static inline void arch_kernel_init(void)
{
	/* Initialize VM interface */
}

/* IRQ lock/unlock functions */
static inline unsigned int arch_irq_lock(void)
{
	unsigned int key;

	__asm__ volatile("r1 = vmgetie()\n"
			 "vmsetie(r0)\n"
			 "r0 = r1\n"
			 : "=r"(key)
			 :
			 : "r1", "memory");

	return key;
}

static inline void arch_irq_unlock(unsigned int key)
{
	__asm__ volatile("vmsetie(r0)\n" : : "r"(key) : "memory");
}

static inline bool arch_irq_unlocked(unsigned int key)
{
	return (key & 1) != 0;
}

/* No-op instruction */
static inline void arch_nop(void)
{
	__asm__ volatile("nop");
}

/* Check if we're in an interrupt context */
static inline bool arch_is_in_isr(void)
{
	return false; /* TODO: Implement based on context */
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_ARCH_H_ */
