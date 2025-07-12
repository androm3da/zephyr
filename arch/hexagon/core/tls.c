/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon Thread-Local Storage (TLS) support
 *
 * Implements TLS according to the Hexagon Application Binary Interface
 * using the UGP (User Global Pointer) register.
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>

/* TLS support for Hexagon using UGP register */

/**
 * Initialize TLS for a thread
 * @param thread Thread to initialize TLS for
 * @param tls_area Pointer to TLS area
 * @param tls_size Size of TLS area
 */
void arch_tls_init(struct k_thread *thread, void *tls_area, size_t tls_size)
{
	if (tls_area == NULL || tls_size == 0) {
		thread->arch.tls_ptr = NULL;
		thread->arch.context.ugp = 0;
		return;
	}

	/* Store TLS pointer in thread structure */
	thread->arch.tls_ptr = tls_area;

	/* According to Hexagon ABI, UGP points to the TLS area */
	/* The TLS area layout follows standard ELF TLS conventions */
	thread->arch.context.ugp = (uint32_t)tls_area;
}

/**
 * Get current thread's TLS pointer
 * @return Pointer to current thread's TLS area
 */
void *arch_tls_get_ptr(void)
{
	struct k_thread *current = k_current_get();
	return current->arch.tls_ptr;
}

/**
 * Switch TLS context during thread switch
 * @param old_thread Previous thread
 * @param new_thread New thread
 */
void hexagon_tls_switch(struct k_thread *old_thread, struct k_thread *new_thread)
{
	/* Save UGP from current thread if it was modified */
	if (old_thread != NULL) {
		uint32_t current_ugp;
		__asm__ volatile("%[current_ugp] = ugp" : [current_ugp] "=r"(current_ugp));
		old_thread->arch.context.ugp = current_ugp;
	}

	/* Load UGP for new thread */
	if (new_thread != NULL && new_thread->arch.context.ugp != 0) {
		__asm__ volatile("ugp = %[ugp]" : : [ugp] "r"(new_thread->arch.context.ugp));
	}
}

/**
 * Allocate TLS area for a thread
 * @param thread Thread to allocate TLS for
 * @param size Size of TLS area to allocate
 * @return 0 on success, negative error code on failure
 */
int arch_tls_alloc(struct k_thread *thread, size_t size)
{
	void *tls_area;

	if (size == 0) {
		return 0;
	}

	/* Allocate TLS area */
	tls_area = k_malloc(size);
	if (tls_area == NULL) {
		return -ENOMEM;
	}

	/* Initialize to zero */
	memset(tls_area, 0, size);

	/* Set up TLS for the thread */
	arch_tls_init(thread, tls_area, size);

	return 0;
}

/**
 * Free TLS area for a thread
 * @param thread Thread to free TLS for
 */
void arch_tls_free(struct k_thread *thread)
{
	if (thread->arch.tls_ptr != NULL) {
		k_free(thread->arch.tls_ptr);
		thread->arch.tls_ptr = NULL;
		thread->arch.context.ugp = 0;
	}
}

/* TLS access functions following Hexagon ABI */

/**
 * Get pointer to thread-local variable by offset
 * @param offset Offset within TLS area
 * @return Pointer to thread-local variable
 */
void *__tls_get_addr(size_t offset)
{
	struct k_thread *current = k_current_get();

	if (current->arch.tls_ptr == NULL) {
		return NULL;
	}

	return (char *)current->arch.tls_ptr + offset;
}

/**
 * Get TLS base pointer (UGP value)
 * @return TLS base pointer
 */
void *__tls_get_base(void)
{
	uint32_t ugp_value;
	__asm__ volatile("%[ugp_value] = ugp" : [ugp_value] "=r"(ugp_value));
	return (void *)ugp_value;
}
