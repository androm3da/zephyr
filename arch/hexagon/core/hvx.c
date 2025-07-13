/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/logging/log.h>
#include <hvx.h>
#include <hexagon_vm.h>

LOG_MODULE_REGISTER(hvx, CONFIG_ARCH_LOG_LEVEL);

#ifdef CONFIG_HEXAGON_HVX

/* HVX subsystem global state */
static struct hvx_config hvx_global_config;
static atomic_t hvx_context_allocation_mask;
static K_MUTEX_DEFINE(hvx_mutex);

/* Memory slab for HVX vector register storage */
K_MEM_SLAB_DEFINE_STATIC(hvx_vectors_slab, sizeof(struct hvx_vectors),
			 CONFIG_HEXAGON_HVX_MAX_CONTEXTS, 4);

/* Per-CPU HVX hardware state */
struct hvx_cpu_state {
	uint8_t current_context;
	uint8_t hardware_xa;
	uint8_t hardware_xe;
	bool hvx_enabled;
};

static struct hvx_cpu_state hvx_cpu_state;

/**
 * @brief Get HVX context from current thread
 */
static inline struct hvx_context *hvx_get_thread_context(void)
{
	return (struct hvx_context *)k_thread_custom_data_get();
}

/**
 * @brief Set HVX context for current thread
 */
static inline void hvx_set_thread_context(struct hvx_context *ctx)
{
	k_thread_custom_data_set((void *)ctx);
}

/**
 * @brief Allocate HVX context number
 */
static int hvx_alloc_context_num(void)
{
	int context_num;
	atomic_t old_mask, new_mask;

	do {
		old_mask = atomic_get(&hvx_context_allocation_mask);

		/* Find first available context */
		context_num = 0;
		for (int i = 0; i < CONFIG_HEXAGON_HVX_MAX_CONTEXTS; i++) {
			if ((old_mask & BIT(i)) == 0) {
				context_num = i;
				break;
			}
		}
		if (context_num >= CONFIG_HEXAGON_HVX_MAX_CONTEXTS) {
			context_num = CONFIG_HEXAGON_HVX_MAX_CONTEXTS;
		}

		if (context_num >= CONFIG_HEXAGON_HVX_MAX_CONTEXTS) {
			return -ENOMEM;
		}

		new_mask = old_mask | BIT(context_num);
	} while (atomic_cas(&hvx_context_allocation_mask, old_mask, new_mask) != old_mask);

	return context_num;
}

/**
 * @brief Free HVX context number
 */
static void hvx_free_context_num(int context_num)
{
	if (context_num >= 0 && context_num < CONFIG_HEXAGON_HVX_MAX_CONTEXTS) {
		atomic_and(&hvx_context_allocation_mask, ~BIT(context_num));
	}
}

/**
 * @brief Set hardware configuration for HVX
 */
static void hvx_configure_hardware(uint8_t vlength, uint8_t extbits)
{
	struct hvx_cpu_state *cpu_state = &hvx_cpu_state;

	if (cpu_state->hardware_xa != extbits) {
		cpu_state->hardware_xa = extbits;
		cpu_state->hardware_xe = (extbits != 0) ? 1 : 0;

		/* Call assembly function to configure hardware */
		hvx_set_hardware_config(vlength, extbits);

		LOG_DBG("HVX hardware configured: vlength=%d, extbits=%d", vlength, extbits);
	}
}

/**
 * @brief Convert context number to XA value
 */
static inline uint8_t hvx_context_to_xa(int context_num)
{
	return 4 + context_num;
}

int hvx_init(void)
{
	LOG_INF("Initializing HVX subsystem");

	/* Initialize global configuration */
	hvx_global_config.max_contexts = CONFIG_HEXAGON_HVX_MAX_CONTEXTS;
	hvx_global_config.vector_length_bytes = CONFIG_HEXAGON_HVX_128B ? 128 : 64;
	hvx_global_config.hvx_enabled = true;

	/* Initialize context allocation mask */
	atomic_set(&hvx_context_allocation_mask, 0);

	/* Configure hardware for initial state (HVX disabled) */
	hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);

	LOG_INF("HVX initialized: max_contexts=%d, vector_length_bytes=%d",
		hvx_global_config.max_contexts, hvx_global_config.vector_length_bytes);

	return 0;
}

int hvx_context_alloc(void)
{
	struct hvx_context *ctx;
	void *vregs_mem;
	int context_num;
	int ret;

	k_mutex_lock(&hvx_mutex, K_FOREVER);

	/* Check if thread already has HVX context */
	ctx = hvx_get_thread_context();
	if (ctx != NULL) {
		k_mutex_unlock(&hvx_mutex);
		return 0; /* Already allocated */
	}

	/* Allocate context number */
	context_num = hvx_alloc_context_num();
	if (context_num < 0) {
		LOG_WRN("No HVX contexts available");
		ret = context_num;
		goto unlock;
	}

	/* Allocate memory for vector registers */
	ret = k_mem_slab_alloc(&hvx_vectors_slab, &vregs_mem, K_NO_WAIT);
	if (ret != 0) {
		LOG_ERR("Failed to allocate HVX vector memory: %d", ret);
		hvx_free_context_num(context_num);
		goto unlock;
	}

	/* Allocate and initialize context structure */
	ctx = k_malloc(sizeof(struct hvx_context));
	if (ctx == NULL) {
		LOG_ERR("Failed to allocate HVX context structure");
		k_mem_slab_free(&hvx_vectors_slab, vregs_mem);
		hvx_free_context_num(context_num);
		ret = -ENOMEM;
		goto unlock;
	}

	/* Initialize context */
	ctx->vregs = (struct hvx_vectors *)vregs_mem;
	ctx->generation = 1;
	ctx->context_num = context_num;
	ctx->prev_context_num = -1;
	ctx->context_valid = false;
	ctx->context_dirty = false;

	/* Clear vector registers */
	memset(ctx->vregs, 0, sizeof(struct hvx_vectors));

	/* Associate context with current thread */
	hvx_set_thread_context(ctx);

	LOG_DBG("HVX context allocated: context_num=%d", context_num);
	ret = 0;

unlock:
	k_mutex_unlock(&hvx_mutex);
	return ret;
}

void hvx_context_free(void)
{
	struct hvx_context *ctx;

	k_mutex_lock(&hvx_mutex, K_FOREVER);

	ctx = hvx_get_thread_context();
	if (ctx == NULL) {
		k_mutex_unlock(&hvx_mutex);
		return; /* No context to free */
	}

	/* Disable HVX for this thread */
	hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);

	/* Free context number */
	hvx_free_context_num(ctx->context_num);

	/* Free vector register memory */
	k_mem_slab_free(&hvx_vectors_slab, ctx->vregs);

	/* Free context structure */
	k_free(ctx);

	/* Remove context from thread */
	hvx_set_thread_context(NULL);

	LOG_DBG("HVX context freed");

	k_mutex_unlock(&hvx_mutex);
}

int hvx_context_save(void)
{
	struct hvx_context *ctx;

	ctx = hvx_get_thread_context();
	if (ctx == NULL || !ctx->context_valid) {
		return 0; /* No context to save */
	}

	if (ctx->context_dirty) {
		/* Save vector registers using assembly function */
		hvx_save_context_asm(ctx->vregs);
		ctx->generation++;
		ctx->context_dirty = false;

		LOG_DBG("HVX context saved: context_num=%d, generation=%u", ctx->context_num,
			ctx->generation);
	}

	return 0;
}

int hvx_context_restore(void)
{
	struct hvx_context *ctx;
	uint8_t xa_value;

	ctx = hvx_get_thread_context();
	if (ctx == NULL) {
		/* No HVX context for this thread, disable HVX */
		hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);
		return 0;
	}

	/* Validate context structure */
	if (ctx->vregs == NULL) {
		LOG_ERR("HVX context has NULL vregs pointer");
		return -EINVAL;
	}

	/* Calculate XA value for this context */
	xa_value = hvx_context_to_xa(ctx->context_num);

	/* Enable HVX with correct context */
	hvx_configure_hardware(hvx_global_config.vector_length_bytes, xa_value);

	if (!ctx->context_valid) {
		/* First time using this context, initialize with zeros */
		/* Use a smaller memset to avoid potential memory issues */
		if (ctx->vregs != NULL) {
			memset(ctx->vregs, 0, sizeof(struct hvx_vectors));
			ctx->context_valid = true;
		} else {
			LOG_ERR("Cannot initialize NULL vregs");
			return -EINVAL;
		}
	}

	/* Restore vector registers using assembly function */
	hvx_restore_context_asm(ctx->vregs);
	ctx->context_dirty = true;

	LOG_DBG("HVX context restored: context_num=%d, generation=%u", ctx->context_num,
		ctx->generation);

	return 0;
}

int hvx_enable(void)
{
	int ret;

	/* Allocate HVX context if not already done */
	ret = hvx_context_alloc();
	if (ret != 0) {
		return ret;
	}

	/* Restore context to enable HVX */
	return hvx_context_restore();
}

void hvx_disable(void)
{
	/* Save current context */
	hvx_context_save();

	/* Disable HVX hardware */
	hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);
}

bool hvx_is_available(void)
{
	return hvx_global_config.hvx_enabled;
}

struct hvx_context *hvx_get_current_context(void)
{
	return hvx_get_thread_context();
}

/**
 * @brief HVX context switch hook for thread switching
 *
 * This function is called during thread context switches to save/restore
 * HVX contexts appropriately. Preemptive context switching and lazy save
 * behavior are always enabled.
 */
void hvx_thread_switch(struct k_thread *old_thread, struct k_thread *new_thread)
{
	/* Save HVX context for outgoing thread */
	if (old_thread != NULL) {
		/* TODO: Get HVX context from old_thread and save if needed */
	}

	/* Restore HVX context for incoming thread */
	if (new_thread != NULL) {
		/* TODO: Get HVX context from new_thread and restore if needed */
	}
}

/* Initialize HVX subsystem at boot time */
SYS_INIT(hvx_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif /* CONFIG_HEXAGON_HVX */
