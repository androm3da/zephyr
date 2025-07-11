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
			 "%[old_value] = memw_locked(%[target])\n\t"      /* Load linked */
			 "%[new_value] = add(%[old_value], %[value])\n\t" /* Add value */
			 "memw_locked(%[target], p0) = %[new_value]\n\t"  /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"                        /* Retry if failed */
			 : [old_value] "=&r"(old_value), [new_value] "=&r"(new_value)
			 : [target] "r"(target), [value] "r"(value)
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
			 "%[old_value] = memw_locked(%[target])\n\t" /* Load linked */
			 "memw_locked(%[target], p0) = %[value]\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"                   /* Retry if failed */
			 : [old_value] "=&r"(old_value)
			 : [target] "r"(target), [value] "r"(value)
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
			 "%[old_value] = memw_locked(%[target])\n\t"     /* Load linked */
			 "%[new_value] = or(%[old_value], %[value])\n\t" /* OR value */
			 "memw_locked(%[target], p0) = %[new_value]\n\t" /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"                       /* Retry if failed */
			 : [old_value] "=&r"(old_value), [new_value] "=&r"(new_value)
			 : [target] "r"(target), [value] "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_xor(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%[old_value] = memw_locked(%[target])\n\t"      /* Load linked */
			 "%[new_value] = xor(%[old_value], %[value])\n\t" /* XOR value */
			 "memw_locked(%[target], p0) = %[new_value]\n\t"  /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"                        /* Retry if failed */
			 : [old_value] "=&r"(old_value), [new_value] "=&r"(new_value)
			 : [target] "r"(target), [value] "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_and(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%[old_value] = memw_locked(%[target])\n\t"      /* Load linked */
			 "%[new_value] = and(%[old_value], %[value])\n\t" /* AND value */
			 "memw_locked(%[target], p0) = %[new_value]\n\t"  /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"                        /* Retry if failed */
			 : [old_value] "=&r"(old_value), [new_value] "=&r"(new_value)
			 : [target] "r"(target), [value] "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE atomic_val_t arch_atomic_nand(atomic_t *target, atomic_val_t value)
{
	atomic_val_t old_value, new_value;

	__asm__ volatile("1:\n\t"
			 "%[old_value] = memw_locked(%[target])\n\t"      /* Load linked */
			 "%[new_value] = and(%[old_value], %[value])\n\t" /* AND value */
			 "%[new_value] = not(%[new_value])\n\t"           /* NOT result */
			 "memw_locked(%[target], p0) = %[new_value]\n\t"  /* Store conditional */
			 "if (!p0) jump:nt 1b\n\t"                        /* Retry if failed */
			 : [old_value] "=&r"(old_value), [new_value] "=&r"(new_value)
			 : [target] "r"(target), [value] "r"(value)
			 : "p0", "memory");

	return old_value;
}

static ALWAYS_INLINE bool arch_atomic_cas(atomic_t *target, atomic_val_t old_value,
					  atomic_val_t new_value)
{
	atomic_val_t current;
	bool success;

	__asm__ volatile(
		"1:\n\t"
		"%[current] = memw_locked(%[target])\n\t"       /* Load linked */
		"p0 = cmp.eq(%[current], %[old_value])\n\t"     /* Compare with expected */
		"if (!p0) jump 2f\n\t"                          /* Skip if not equal */
		"memw_locked(%[target], p0) = %[new_value]\n\t" /* Store conditional */
		"if (!p0) jump:nt 1b\n\t"                       /* Retry if failed */
		"2:\n\t"
		"%[success] = p0\n\t" /* Return success flag */
		: [current] "=&r"(current), [success] "=r"(success)
		: [target] "r"(target), [old_value] "r"(old_value), [new_value] "r"(new_value)
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
