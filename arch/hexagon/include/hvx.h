/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_HVX_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_HVX_H_

#include <zephyr/types.h>
#include <zephyr/arch/cpu.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HVX (Hexagon Vector Extensions) Support
 * @defgroup hexagon_hvx Hexagon HVX
 * @ingroup hexagon_arch
 * @{
 */

#ifdef CONFIG_HEXAGON_HVX

/* HVX Vector size - configurable 64 or 128 bytes */
#ifdef CONFIG_HEXAGON_HVX_128B
#define HVX_VECTOR_SIZE 128
#else
#define HVX_VECTOR_SIZE 64
#endif

/* HVX Vector data types */
typedef uint8_t hvx_vector_t[HVX_VECTOR_SIZE] __aligned(HVX_VECTOR_SIZE);
typedef uint8_t hvx_vector_pred_t[HVX_VECTOR_SIZE] __aligned(HVX_VECTOR_SIZE);

/**
 * @brief HVX Vector register set
 *
 * Contains all 32 HVX vector registers (V0-V31) plus vector predicate registers
 */
struct hvx_vectors {
	hvx_vector_t v[32];       /* V0-V31 vector registers */
	hvx_vector_t vecpredsave; /* Vector predicate save register */
};

/**
 * @brief HVX context information per thread
 */
struct hvx_context {
	struct hvx_vectors *vregs; /* Pointer to saved vector registers */
	uint32_t generation;       /* Context generation counter */
	int8_t context_num;        /* HVX context number (-1 if not allocated) */
	int8_t prev_context_num;   /* Previous context number */
	bool context_valid;        /* True if context contains valid data */
	bool context_dirty;        /* True if context needs to be saved */
};

/**
 * @brief HVX hardware configuration
 */
struct hvx_config {
	uint8_t max_contexts;  /* Maximum number of HVX contexts */
	uint8_t vector_length_bytes; /* Vector length configuration */
	bool hvx_enabled;      /* HVX subsystem enabled */
};

/* HVX hardware configuration fields */
#define HVX_HWCONFIG_VLENGTH_SHIFT 0
#define HVX_HWCONFIG_VLENGTH_MASK  0x7
#define HVX_HWCONFIG_EXTBITS_SHIFT 8
#define HVX_HWCONFIG_EXTBITS_MASK  0xFF

/* HVX context allocation flags */
#define HVX_CONTEXT_AVAILABLE 0
#define HVX_CONTEXT_ALLOCATED 1

/**
 * @brief Initialize HVX subsystem
 * @return 0 on success, negative error code on failure
 */
int hvx_init(void);

/**
 * @brief Allocate HVX context for current thread
 * @return 0 on success, negative error code on failure
 */
int hvx_context_alloc(void);

/**
 * @brief Free HVX context for current thread
 */
void hvx_context_free(void);

/**
 * @brief Save HVX context for current thread
 * @return 0 on success, negative error code on failure
 */
int hvx_context_save(void);

/**
 * @brief Restore HVX context for current thread
 * @return 0 on success, negative error code on failure
 */
int hvx_context_restore(void);

/**
 * @brief Enable HVX for current thread
 *
 * This function enables HVX usage for the current thread by configuring
 * the hardware and allocating necessary resources.
 *
 * @return 0 on success, negative error code on failure
 */
int hvx_enable(void);

/**
 * @brief Disable HVX for current thread
 */
void hvx_disable(void);

/**
 * @brief Check if HVX is available on this hardware
 * @return true if HVX is available, false otherwise
 */
bool hvx_is_available(void);

/**
 * @brief Get HVX context for current thread
 * @return Pointer to HVX context or NULL if not allocated
 */
struct hvx_context *hvx_get_current_context(void);

/* Low-level assembly functions */
extern void hvx_save_context_asm(struct hvx_vectors *vregs);
extern void hvx_restore_context_asm(struct hvx_vectors *vregs);

/* Hardware configuration functions */
extern void hvx_set_hardware_config(uint8_t vlength, uint8_t extbits);
extern uint32_t hvx_get_hardware_config(void);

#else /* !CONFIG_HEXAGON_HVX */

/* Stub functions when HVX is disabled */
static inline int hvx_init(void)
{
	return 0;
}
static inline int hvx_context_alloc(void)
{
	return -ENOTSUP;
}
static inline void hvx_context_free(void)
{
}
static inline int hvx_context_save(void)
{
	return 0;
}
static inline int hvx_context_restore(void)
{
	return 0;
}
static inline int hvx_enable(void)
{
	return -ENOTSUP;
}
static inline void hvx_disable(void)
{
}
static inline bool hvx_is_available(void)
{
	return false;
}
static inline struct hvx_context *hvx_get_current_context(void)
{
	return NULL;
}

#endif /* CONFIG_HEXAGON_HVX */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_HVX_H_ */
