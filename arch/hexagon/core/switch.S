/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

/*
 * Hexagon context switching implementation
 * Based on Linux kernel vm_switch.S
 */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>
#include <zephyr/arch/hexagon/thread.h>

/* Stack frame offsets for context switch */
#define SWITCH_R1716     0x00
#define SWITCH_R1918     0x08
#define SWITCH_R2120     0x10
#define SWITCH_R2322     0x18
#define SWITCH_R2524     0x20
#define SWITCH_R2726     0x28
#define SWITCH_FP        0x30
#define SWITCH_LR        0x38
#ifdef CONFIG_THREAD_LOCAL_STORAGE
#define SWITCH_FRAME_SIZE 0x48  /* Include space for UGP */
#else
#define SWITCH_FRAME_SIZE 0x40
#endif

GTEXT(z_hexagon_arch_switch)

/*
 * z_hexagon_arch_switch(void *switch_to, void **switched_from)
 *
 * switch_to (r0): Pointer to new thread
 * switched_from (r1): Pointer to switch_handle field of old thread
 *
 * This function performs the low-level context switch using Hexagon
 * callee-saved registers and stack switching.
 */
SECTION_FUNC(TEXT, z_hexagon_arch_switch)
	/* Allocate stack frame for saving context */
	allocframe(#SWITCH_FRAME_SIZE)

	/* Save callee-saved registers to stack */
	{
		memd(r29+#SWITCH_R1716) = r17:16
		memd(r29+#SWITCH_R1918) = r19:18
	}
	{
		memd(r29+#SWITCH_R2120) = r21:20
		memd(r29+#SWITCH_R2322) = r23:22
	}
	{
		memd(r29+#SWITCH_R2524) = r25:24
		memd(r29+#SWITCH_R2726) = r27:26
	}
	{
		memd(r29+#SWITCH_FP) = r31:30
	}

	/* Store current stack pointer as switch handle for old thread */
	{
		memw(r1) = r29
	}

	/* Save current UGP (TLS pointer) if TLS is enabled */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	{
		r2 = ugp
		memw(r29+#SWITCH_FRAME_SIZE) = r2  /* Store UGP after frame */
	}
#endif

	/* Load new thread's switch handle */
	/* switch_to is the switch handle value for the thread */
	{
		r29 = r0  /* Use switch_to directly as the stack pointer */
	}

	/* Check if this is a new thread (switch_to points to callee_saved) */
	/* Always assume this is a new thread for now to debug the issue */
	/* TODO: Add proper detection later */
	jump new_thread_setup

existing_thread_restore:

	/* Restore callee-saved registers from new thread's stack */
	{
		r17:16 = memd(r29+#SWITCH_R1716)
		r19:18 = memd(r29+#SWITCH_R1918)
	}
	{
		r21:20 = memd(r29+#SWITCH_R2120)
		r23:22 = memd(r29+#SWITCH_R2322)
	}
	{
		r25:24 = memd(r29+#SWITCH_R2524)
		r27:26 = memd(r29+#SWITCH_R2726)
	}
	{
		r31:30 = memd(r29+#SWITCH_FP)
	}

	/* Restore UGP (TLS pointer) if TLS is enabled */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	{
		r2 = memw(r29+#SWITCH_FRAME_SIZE)
		ugp = r2
	}
#endif

	/* Return to the new thread */
	{
		dealloc_return
	}

new_thread_setup:
	/* For new threads, r29 points to callee_saved structure */
	/* Load the actual stack pointer */
	{
		r2 = memw(r29+#((12*4)+4))  /* Load r30_sp from callee_saved */
		r3 = memw(r29+#((12*4)+8))  /* Load r31_lr from callee_saved */
	}
	/* Load thread parameters from callee_saved into correct registers for z_thread_entry */
	{
		r0 = memw(r29+#(0*4))   /* Entry function -> r0 */
		r1 = memw(r29+#(1*4))   /* p1 -> r1 */
	}
	{
		r2 = memw(r29+#(2*4))   /* p2 -> r2 */
		r3 = memw(r29+#(3*4))   /* p3 -> r3 */
	}

	/* Load the actual stack pointer and thread entry address */
	{
		r4 = memw(r29+#((12*4)+4))  /* Load r30_sp from callee_saved */
		r5 = memw(r29+#((12*4)+8))  /* Load r31_lr from callee_saved */
	}

	/* Switch to the new thread's stack */
	{
		r29 = r4  /* Use actual stack pointer */
		r30 = r4  /* Set frame pointer */
	}

	/* Set up UGP (TLS pointer) for new thread if TLS is enabled */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TODO: Load UGP from thread structure if TLS is configured */
	/* For now, initialize to 0 for new threads */
	{
		ugp = #0
	}
#endif

	/* Jump to z_thread_entry - this is FUNC_NORETURN so don't use callr */
	{
		jumpr r5
	}
