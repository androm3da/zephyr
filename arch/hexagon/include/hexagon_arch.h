/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_ARCH_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_ARCH_H_

/**
 * @file
 * @brief Hexagon architectural features and control registers
 *
 * This file defines architectural features of the Hexagon processor
 * including control registers, system registers, and architectural
 * instructions that are not part of the VM interface.
 */

#ifndef _ASMLANGUAGE
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hexagon Control Registers */

/**
 * @brief Get FRAMELIMIT register (c16)
 *
 * The FRAMELIMIT register defines the lowest valid stack address.
 * Any stack access below this limit triggers a hardware exception.
 *
 * @return Current FRAMELIMIT value
 */
static inline uintptr_t hexagon_get_framelimit(void)
{
	uintptr_t result;

	__asm__ volatile("%[result] = c16" : [result] "=r"(result) : : "memory");
	return result;
}

/**
 * @brief Set FRAMELIMIT register (c16)
 *
 * Sets the stack overflow protection limit. Any stack access below
 * this address will trigger a hardware exception.
 *
 * @param limit Stack limit address
 */
static inline void hexagon_set_framelimit(uintptr_t limit)
{
	__asm__ volatile("c16 = %[limit]" : : [limit] "r"(limit) : "memory");
}

/**
 * @brief Get current stack pointer (r29)
 *
 * @return Current stack pointer value
 */
static inline uintptr_t hexagon_get_stack_pointer(void)
{
	uintptr_t result;

	__asm__ volatile("%[result] = r29" : [result] "=r"(result));
	return result;
}

/**
 * @brief Get frame pointer (r30)
 *
 * @return Current frame pointer value
 */
static inline uintptr_t hexagon_get_frame_pointer(void)
{
	uintptr_t result;

	__asm__ volatile("%[result] = r30" : [result] "=r"(result));
	return result;
}

/* User Status Register (USR) operations */

/**
 * @brief Get User Status Register
 *
 * @return Current USR value
 */
static inline uint32_t hexagon_get_usr(void)
{
	uint32_t result;

	__asm__ volatile("%[result] = usr" : [result] "=r"(result));
	return result;
}

/**
 * @brief Set User Status Register
 *
 * @param usr_value New USR value
 */
static inline void hexagon_set_usr(uint32_t usr_value)
{
	__asm__ volatile("usr = %[usr_value]" : : [usr_value] "r"(usr_value));
}

/* FRAMEKEY Register Operations for Return Address Protection */

/**
 * @brief Get FRAMEKEY register
 *
 * The FRAMEKEY register is used for stack smashing protection by
 * scrambling return addresses stored on the stack. In allocframe,
 * the return address is XOR-scrambled with FRAMEKEY before storage.
 * In deallocframe/dealloc_return, it's unscrambled before loading to LR.
 *
 * @return Current FRAMEKEY value
 */
static inline uint32_t hexagon_get_framekey(void)
{
	uint32_t result;

	__asm__ volatile("%[result] = framekey" : [result] "=r"(result) : : "memory");
	return result;
}

/**
 * @brief Set FRAMEKEY register
 *
 * Sets the frame key used for return address scrambling. After reset,
 * FRAMEKEY defaults to 0 (protection disabled). A non-zero value
 * enables stack smashing protection by scrambling return addresses.
 *
 * @param key Frame key value for return address scrambling
 */
static inline void hexagon_set_framekey(uint32_t key)
{
	__asm__ volatile("framekey = %[key]" : : [key] "r"(key) : "memory");
}

/* Interrupt controller functions */

/**
 * @brief Trigger a software interrupt
 *
 * Posts a software interrupt to the specified IRQ number using
 * Hexagon VM hypercalls.
 *
 * @param irq IRQ number to trigger
 * @return 0 on success, -1 on error
 */
int hexagon_irq_trigger(unsigned int irq);

/**
 * @brief Initialize interrupt controller
 *
 * Initializes the Hexagon interrupt controller using VM hypercalls.
 * Disables all interrupts initially.
 */
void hexagon_intc_init(void);

/**
 * @brief Enable interrupt
 *
 * Enables the specified interrupt using VM hypercalls.
 *
 * @param irq IRQ number to enable
 */
void hexagon_intc_enable(uint32_t irq);

/**
 * @brief Disable interrupt
 *
 * Disables the specified interrupt using VM hypercalls.
 *
 * @param irq IRQ number to disable
 */
void hexagon_intc_disable(uint32_t irq);

/**
 * @brief Set interrupt priority
 *
 * Sets interrupt priority. Note: VM operations don't support
 * priority setting, so this is a compatibility stub.
 *
 * @param irq IRQ number
 * @param priority Priority value (unused)
 */
void hexagon_intc_set_priority(uint32_t irq, uint32_t priority);

/**
 * @brief Get pending interrupt
 *
 * Gets the highest priority pending interrupt using VM hypercalls.
 *
 * @return IRQ number or HEXAGON_NO_PENDING_IRQ if none pending
 */
uint32_t hexagon_intc_get_pending(void);

/**
 * @brief Acknowledge interrupt
 *
 * Acknowledges (EOI) the specified interrupt using VM hypercalls.
 *
 * @param irq IRQ number to acknowledge
 */
void hexagon_intc_ack(uint32_t irq);

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_HEXAGON_ARCH_H_ */
