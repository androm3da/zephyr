/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
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

/*
 * Hardware supports at most 8 HVX contexts.  The actual limit for this
 * target is discovered at boot by querying the Hexagon VM.
 */
#define HVX_MAX_CONTEXTS_HW 8

/* HVX subsystem global state */
static struct hvx_config hvx_global_config;
static atomic_t hvx_context_allocation_mask;
static K_MUTEX_DEFINE(hvx_mutex);

/* Memory slab for HVX vector register storage */
K_MEM_SLAB_DEFINE_STATIC(hvx_vectors_slab, sizeof(struct hvx_vectors),
			 HVX_MAX_CONTEXTS_HW, 4);

/* Per-CPU HVX hardware state.
 *
 * NOTE: This is a single static instance.  Under SMP it must become per-CPU
 * data (e.g. using K_PER_CPU).  For now, Hexagon is always single-CPU.
 * Add BUILD_ASSERT(!IS_ENABLED(CONFIG_SMP)) here if SMP support is added.
 */
struct hvx_cpu_state {
	uint8_t hardware_xa;
	bool hvx_enabled;
};

static struct hvx_cpu_state hvx_cpu_state;

/*
 * Per-thread HVX context accessors.
 *
 * The context pointer is stored in _thread_arch.hvx_ctx rather than
 * k_thread_custom_data so it does not conflict with application use of the
 * custom-data slot.
 */
static inline struct hvx_context *hvx_get_thread_context(void)
{
	return _current->arch.hvx_ctx;
}

static inline void hvx_set_thread_context(struct hvx_context *ctx)
{
	_current->arch.hvx_ctx = ctx;
}

static int hvx_alloc_context_num(void)
{
	int context_num;
	atomic_t old_mask, new_mask;

	/*
	 * This function is only called from hvx_context_alloc() which holds
	 * hvx_mutex, so the CAS loop always succeeds on the first iteration.
	 * The CAS is kept here to allow potential future lock-free callers
	 * without requiring interface changes.
	 */
	do {
		old_mask = atomic_get(&hvx_context_allocation_mask);

		/* Find first available context */
		context_num = -1;
		for (int i = 0; i < hvx_global_config.max_contexts; i++) {
			if ((old_mask & BIT(i)) == 0) {
				context_num = i;
				break;
			}
		}

		if (context_num < 0) {
			return -ENOMEM;
		}

		new_mask = old_mask | BIT(context_num);
	} while (!atomic_cas(&hvx_context_allocation_mask, old_mask, new_mask));

	return context_num;
}

static void hvx_free_context_num(int context_num)
{
	if (context_num >= 0 && context_num < hvx_global_config.max_contexts) {
		atomic_and(&hvx_context_allocation_mask, ~BIT(context_num));
	}
}

static void hvx_configure_hardware(uint8_t vlength, uint8_t extbits)
{
	struct hvx_cpu_state *cpu_state = &hvx_cpu_state;

	if (cpu_state->hardware_xa != extbits) {
		cpu_state->hardware_xa = extbits;

		/* Call assembly function to configure hardware */
		hvx_set_hardware_config(vlength, extbits);

		LOG_DBG("HVX hardware configured: vlength=%d, extbits=%d", vlength, extbits);
	}
}

static inline uint8_t hvx_context_to_xa(int context_num)
{
	return 4 + context_num;
}

int hvx_init(void)
{
	uint32_t vm_max;

	LOG_INF("Initializing HVX subsystem");

	/* Query the Hexagon VM for the number of HVX contexts available */
	vm_max = hexagon_vm_getinfo(vm_info_hvx_contexts);
	if (vm_max == 0 || vm_max > HVX_MAX_CONTEXTS_HW) {
		vm_max = HVX_MAX_CONTEXTS_HW;
	}

	/* Initialize global configuration */
	hvx_global_config.max_contexts = vm_max;
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

	/*
	 * hvx_mutex is held across k_malloc() below.  k_malloc() does not
	 * block on a normal heap, but if the heap is backed by a pool that
	 * can block (e.g. CONFIG_HEAP_LISTENER + a compact-on-OOM path) and
	 * a memory-freeing thread also tries to acquire hvx_mutex, a deadlock
	 * could occur.  On the current single-pool heap this is benign; note
	 * it here in case the allocator changes.
	 */
	k_mutex_lock(&hvx_mutex, K_FOREVER);

	ctx = hvx_get_thread_context();
	if (ctx != NULL) {
		k_mutex_unlock(&hvx_mutex);
		return 0;
	}

	context_num = hvx_alloc_context_num();
	if (context_num < 0) {
		LOG_WRN("No HVX contexts available");
		ret = context_num;
		goto unlock;
	}

	ret = k_mem_slab_alloc(&hvx_vectors_slab, &vregs_mem, K_NO_WAIT);
	if (ret != 0) {
		LOG_ERR("Failed to allocate HVX vector memory: %d", ret);
		hvx_free_context_num(context_num);
		goto unlock;
	}

	ctx = k_malloc(sizeof(struct hvx_context));
	if (ctx == NULL) {
		LOG_ERR("Failed to allocate HVX context structure");
		k_mem_slab_free(&hvx_vectors_slab, vregs_mem);
		hvx_free_context_num(context_num);
		ret = -ENOMEM;
		goto unlock;
	}

	ctx->vregs = (struct hvx_vectors *)vregs_mem;
	ctx->generation = 1;
	ctx->context_num = context_num;
	ctx->prev_context_num = -1;
	ctx->context_valid = false;
	ctx->context_dirty = false;

	memset(ctx->vregs, 0, sizeof(struct hvx_vectors));
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
		return;
	}

	hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);
	hvx_free_context_num(ctx->context_num);
	k_mem_slab_free(&hvx_vectors_slab, ctx->vregs);
	k_free(ctx);
	hvx_set_thread_context(NULL);

	LOG_DBG("HVX context freed");

	k_mutex_unlock(&hvx_mutex);
}

int hvx_context_save(void)
{
	struct hvx_context *ctx;

	ctx = hvx_get_thread_context();
	if (ctx == NULL || !ctx->context_valid) {
		return 0;
	}

	if (ctx->context_dirty) {
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
		hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);
		return 0;
	}

	if (ctx->vregs == NULL) {
		LOG_ERR("HVX context has NULL vregs pointer");
		return -EINVAL;
	}

	xa_value = hvx_context_to_xa(ctx->context_num);
	hvx_configure_hardware(hvx_global_config.vector_length_bytes, xa_value);

	if (!ctx->context_valid) {
		memset(ctx->vregs, 0, sizeof(struct hvx_vectors));
		ctx->context_valid = true;
	}

	hvx_restore_context_asm(ctx->vregs);
	ctx->context_dirty = true;

	LOG_DBG("HVX context restored: context_num=%d, generation=%u", ctx->context_num,
		ctx->generation);

	return 0;
}

int hvx_enable(void)
{
	int ret;

	ret = hvx_context_alloc();
	if (ret != 0) {
		return ret;
	}

	return hvx_context_restore();
}

void hvx_disable(void)
{
	hvx_context_save();
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

/*
 * Context-switch hook: save old thread's HVX state, restore new thread's.
 *
 * Called from arch_switch() in kernel_arch_func.h after the stack has been
 * exchanged.  At call time:
 *   - old_thread->arch.hvx_ctx holds the context that was active before the switch.
 *   - new_thread == _current, and new_thread->arch.hvx_ctx holds the context
 *     to restore (or NULL if the new thread has never used HVX).
 *
 * We deliberately save via old_thread->arch.hvx_ctx directly rather than via
 * hvx_context_save(), which reads _current (already updated to new_thread).
 */
void hvx_arch_thread_switch(struct k_thread *old_thread, struct k_thread *new_thread)
{
	struct hvx_context *old_ctx = old_thread->arch.hvx_ctx;
	struct hvx_context *new_ctx = new_thread->arch.hvx_ctx;

	/* Save old thread's HVX state if it has a context and it's dirty */
	if (old_ctx != NULL && old_ctx->context_dirty) {
		hvx_save_context_asm(old_ctx->vregs);
		old_ctx->generation++;
		old_ctx->context_dirty = false;
	}

	/* Restore new thread's HVX state, or disable HVX if thread has none */
	if (new_ctx != NULL) {
		uint8_t xa_value = hvx_context_to_xa(new_ctx->context_num);

		hvx_configure_hardware(hvx_global_config.vector_length_bytes, xa_value);
		if (!new_ctx->context_valid) {
			memset(new_ctx->vregs, 0, sizeof(struct hvx_vectors));
			new_ctx->context_valid = true;
		}
		hvx_restore_context_asm(new_ctx->vregs);
		new_ctx->context_dirty = true;
	} else {
		hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);
	}
}

/*
 * Called from EVENT_EXIT assembly BEFORE z_hexagon_arch_switch to
 * eagerly save the old thread's dirty HVX state.
 */
void z_hexagon_event_exit_hvx_save(struct k_thread *old_thread)
{
	struct hvx_context *old_ctx = old_thread->arch.hvx_ctx;

	if (old_ctx != NULL && old_ctx->context_dirty) {
		hvx_save_context_asm(old_ctx->vregs);
		old_ctx->generation++;
		old_ctx->context_dirty = false;
	}
}

/*
 * Called from EVENT_EXIT assembly AFTER z_hexagon_arch_switch to
 * restore the new thread's HVX context.
 */
void z_hexagon_event_exit_hvx_restore(void)
{
	struct k_thread *new_thread = _current;
	struct hvx_context *new_ctx = new_thread->arch.hvx_ctx;

	if (new_ctx != NULL) {
		uint8_t xa_value = hvx_context_to_xa(new_ctx->context_num);

		hvx_configure_hardware(hvx_global_config.vector_length_bytes, xa_value);
		if (!new_ctx->context_valid) {
			memset(new_ctx->vregs, 0, sizeof(struct hvx_vectors));
			new_ctx->context_valid = true;
		}
		hvx_restore_context_asm(new_ctx->vregs);
		new_ctx->context_dirty = true;
	} else {
		hvx_configure_hardware(hvx_global_config.vector_length_bytes, 0);
	}
}

SYS_INIT(hvx_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif /* CONFIG_HEXAGON_HVX */
