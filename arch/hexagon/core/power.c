/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/**
 * @file
 * @brief Hexagon power management implementation
 *
 * Provides CPU sleep states and power saving functionality using
 * Hexagon VM power management operations.
 */

#include <zephyr/kernel.h>
#include <zephyr/pm/pm.h>
#include <zephyr/arch/hexagon/arch.h>
#include <zephyr/arch/cpu.h>

#ifdef CONFIG_PM

/* Power states supported by Hexagon */
static const struct pm_state_info pm_states[] = {
	{PM_STATE_RUNTIME_IDLE, 0, 0, 0},    /* Active idle */
	{PM_STATE_SUSPEND_TO_IDLE, 0, 0, 0}, /* Light sleep */
	{PM_STATE_STANDBY, 0, 0, 0},         /* Deep sleep */
};

/* Save context before entering low power state */
struct hexagon_pm_context {
	uint32_t int_enable;
	uint32_t timer_state;
	/* Add more context as needed */
};

static struct hexagon_pm_context pm_context;

/* Enter low power state */
void pm_state_set(enum pm_state state, uint8_t substate_id)
{
	ARG_UNUSED(substate_id);

	switch (state) {
	case PM_STATE_RUNTIME_IDLE:
		/* Simple wait state */
		arch_cpu_idle();
		break;

	case PM_STATE_SUSPEND_TO_IDLE:
		/* Light sleep - preserve more context */
		pm_context.int_enable = arch_irq_lock();

		/* Enter wait state using VM operation */
		__asm__ volatile("trap1(#0x10)" ::: "memory"); /* vmwait */

		/* Restore interrupt state */
		arch_irq_unlock(pm_context.int_enable);
		break;

	case PM_STATE_STANDBY:
		/* Deep sleep - full context save */
		pm_context.int_enable = arch_irq_lock();

		/* Prepare for deep sleep */
		__asm__ volatile("trap1(#0x11)" ::: "memory"); /* vmdeepsleep */

		/* Restore context on wake */
		arch_irq_unlock(pm_context.int_enable);
		break;

	default:
		/* Unsupported state */
		break;
	}
}

/* Exit low power state */
void pm_state_exit_post_ops(enum pm_state state, uint8_t substate_id)
{
	ARG_UNUSED(substate_id);

	/* Restore any additional state if needed */
	switch (state) {
	case PM_STATE_STANDBY:
		/* Re-initialize hardware after deep sleep */
		/* TODO: Add hardware re-initialization when needed */
		break;
	default:
		break;
	}

	/* Clear any pending interrupts */
	irq_unlock(0);
}

/* Check if CPU can enter given power state */
bool pm_policy_state_lock_is_active(enum pm_state state, uint8_t substate_id)
{
	ARG_UNUSED(substate_id);

	/* Check if state is locked */
	return false; /* No locks for now */
}

/* Initialize power management */
static int hexagon_pm_init(void)
{
	printk("Power management initialized with %d states\n", ARRAY_SIZE(pm_states));
	return 0;
}

SYS_INIT(hexagon_pm_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif /* CONFIG_PM */

/* CPU power control helpers */
void arch_cpu_sleep(void)
{
#ifdef CONFIG_PM
	pm_state_set(PM_STATE_SUSPEND_TO_IDLE, 0);
#else
	arch_cpu_idle();
#endif
}

void arch_cpu_deep_sleep(void)
{
#ifdef CONFIG_PM
	pm_state_set(PM_STATE_STANDBY, 0);
#else
	arch_cpu_idle();
#endif
}
