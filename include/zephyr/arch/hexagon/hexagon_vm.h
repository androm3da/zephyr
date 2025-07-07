/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hexagon Virtual Machine (HVM) interface definitions
 */

#ifndef ZEPHYR_INCLUDE_ARCH_HEXAGON_HEXAGON_VM_H_
#define ZEPHYR_INCLUDE_ARCH_HEXAGON_HEXAGON_VM_H_

/*
 * HVM trap1 hypercall numbers
 */
#define HVM_TRAP1_VMVERSION	0
#define HVM_TRAP1_VMRTE		1
#define HVM_TRAP1_VMSETVEC	2
#define HVM_TRAP1_VMGETTIME	14
#define HVM_TRAP1_VMSETTIME	15
#define HVM_TRAP1_VMSTART	18
#define HVM_TRAP1_VMSTOP	19
#define HVM_TRAP1_VMGETINFO	26

/*
 * VM operations for timer
 */
enum VM_TIMER_OPS {
	VM_TIMER_STOP = 0,
	VM_TIMER_START = 1,
};

/*
 * VM stop status
 */
enum VM_STOP_STATUS {
	VM_STOP_STATUS_NORMAL = 0,
	VM_STOP_STATUS_ERROR = 1,
};

/*
 * VM info types for VMGETINFO
 */
enum VM_INFO_TYPE {
	VM_INFO_TYPE_VERSION = 0,
	VM_INFO_TYPE_ANGEL_CONSOLE = 1,
};

/*
 * Angel console operations
 */
#define ANGEL_CONSOLE_WRITE	0x01
#define ANGEL_CONSOLE_READ	0x02

/* Angel console structure for HVM communication */
struct angel_console_msg {
	unsigned int op;
	unsigned int len;
	char *buf;
};

#ifndef __ASSEMBLY__

/*
 * HVM hypercall function prototypes
 */

/**
 * @brief Get HVM version
 * @return Version number
 */
static inline long __vmversion(void)
{
	register long ret __asm__("r0");
	__asm__ __volatile__(
		"trap1(#%1)"
		: "=r"(ret)
		: "i"(HVM_TRAP1_VMVERSION)
		: "memory");
	return ret;
}

/**
 * @brief Get VM time
 * @return Current VM time
 */
static inline long long __vmgettime(void)
{
	register long long ret __asm__("r1:0");
	__asm__ __volatile__(
		"trap1(#%1)"
		: "=r"(ret)
		: "i"(HVM_TRAP1_VMGETTIME)
		: "memory");
	return ret;
}

/**
 * @brief Start VM
 * @param start_addr Starting address
 * @return Status
 */
static inline long __vmstart(void *start_addr)
{
	register long ret __asm__("r0");
	register void *addr __asm__("r0") = start_addr;
	__asm__ __volatile__(
		"trap1(#%2)"
		: "=r"(ret)
		: "r"(addr), "i"(HVM_TRAP1_VMSTART)
		: "memory");
	return ret;
}

/**
 * @brief Stop VM
 * @param status Stop status
 * @return Status
 */
static inline long __vmstop(int status)
{
	register long ret __asm__("r0");
	register int stat __asm__("r0") = status;
	__asm__ __volatile__(
		"trap1(#%2)"
		: "=r"(ret)
		: "r"(stat), "i"(HVM_TRAP1_VMSTOP)
		: "memory");
	return ret;
}

/**
 * @brief Get VM info
 * @param type Info type to query
 * @param info Pointer to info structure
 * @return Status
 */
static inline long __vmgetinfo(int type, void *info)
{
	register long ret __asm__("r0");
	register int info_type __asm__("r0") = type;
	register void *info_ptr __asm__("r1") = info;
	__asm__ __volatile__(
		"trap1(#%3)"
		: "=r"(ret)
		: "r"(info_type), "r"(info_ptr), "i"(HVM_TRAP1_VMGETINFO)
		: "memory");
	return ret;
}

#endif /* __ASSEMBLY__ */

#endif /* ZEPHYR_INCLUDE_ARCH_HEXAGON_HEXAGON_VM_H_ */
