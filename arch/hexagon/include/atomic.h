/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#ifndef ZEPHYR_ARCH_HEXAGON_INCLUDE_ATOMIC_H_
#define ZEPHYR_ARCH_HEXAGON_INCLUDE_ATOMIC_H_

#include <zephyr/types.h>
#include <zephyr/sys/atomic.h>

#ifdef CONFIG_ATOMIC_OPERATIONS_ARCH

/* Hexagon atomic operations using LL/SC instructions */

static ALWAYS_INLINE atomic_val_t arch_atomic_add(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%0 = memw_locked(%2)\n\t"     /* Load linked */
			 "%1 = add(%0, %3)\n\t"         /* Add value */
			 "memw_locked(%2, p0) = %1\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"      /* Retry if failed */
			 : "=&r"(old_value), "=&r"(new_value)
			 : "r"(target), "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_sub(atomic_t *target, atomic_val_t value)
{
	return arch_atomic_add(target, -value);
}

static ALWAYS_INLINE atomic_val_t arch_atomic_set(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value;

	__asm__ volatile("1:\n\t"
			 "%0 = memw_locked(%1)\n\t"     /* Load linked */
			 "memw_locked(%1, p0) = %2\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"      /* Retry if failed */
			 : "=&r"(old_value)
			 : "r"(target), "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_get(const atomic_t *target)
{
	/* Simple load is atomic for aligned 32-bit values */
	return *target;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_or(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%0 = memw_locked(%2)\n\t"     /* Load linked */
			 "%1 = or(%0, %3)\n\t"          /* OR value */
			 "memw_locked(%2, p0) = %1\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"      /* Retry if failed */
			 : "=&r"(old_value), "=&r"(new_value)
			 : "r"(target), "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_xor(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%0 = memw_locked(%2)\n\t"     /* Load linked */
			 "%1 = xor(%0, %3)\n\t"         /* XOR value */
			 "memw_locked(%2, p0) = %1\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"      /* Retry if failed */
			 : "=&r"(old_value), "=&r"(new_value)
			 : "r"(target), "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_and(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%0 = memw_locked(%2)\n\t"     /* Load linked */
			 "%1 = and(%0, %3)\n\t"         /* AND value */
			 "memw_locked(%2, p0) = %1\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"      /* Retry if failed */
			 : "=&r"(old_value), "=&r"(new_value)
			 : "r"(target), "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_nand(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%0 = memw_locked(%2)\n\t"     /* Load linked */
			 "%1 = and(%0, %3)\n\t"         /* AND value */
			 "%1 = not(%1)\n\t"             /* NOT result */
			 "memw_locked(%2, p0) = %1\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"      /* Retry if failed */
			 : "=&r"(old_value), "=&r"(new_value)
			 : "r"(target), "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE bool arch_atomic_cas(atomic_t *target, atomic_val_t old_value,
					  atomic_val_t new_value)
{
	atomic_val_t current;
	bool success;

	__asm__ volatile("1:\n\t"
			 "%0 = memw_locked(%2)\n\t"     /* Load linked */
			 "p0 = cmp.eq(%0, %3)\n\t"      /* Compare with expected */
			 "if (!p0) jump 2f\n\t"         /* Skip if not equal */
			 "memw_locked(%2, p0) = %4\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"      /* Retry if failed */
			 "2:\n\t"
			 "%1 = p0\n\t" /* Return success flag */
			 : "=&r"(current), "=r"(success)
			 : "r"(target), "r"(old_value), "r"(new_value)
			 : "p0", "memory");

	return success;
}

#endif /* CONFIG_ATOMIC_OPERATIONS_ARCH */

/* Memory barriers */
static ALWAYS_INLINE void arch_atomic_memory_barrier(void)
{
	__asm__ volatile("barrier" ::: "memory");
}

static ALWAYS_INLINE void arch_atomic_memory_barrier_dsb(void)
{
	__asm__ volatile("syncht" ::: "memory");
}

#endif /* ZEPHYR_ARCH_HEXAGON_INCLUDE_ATOMIC_H_ */
