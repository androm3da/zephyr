/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>

/**
 * @brief HVX Context Switching Assembly Functions
 *
 * Based on Linux kernel implementation but adapted for Zephyr RTOS
 * Requires -mhvx and -mhvx-length compiler flags
 */

.text

/**
 * @brief Save HVX vector registers and predicates
 *
 * void hvx_save_context_asm(struct hvx_vectors *vregs)
 *
 * @param R0 Pointer to hvx_vectors structure
 */
GTEXT(hvx_save_context_asm)
SECTION_FUNC(TEXT, hvx_save_context_asm)
	/* Define temporary vector registers for predicate handling */
#define ZEROS  V0
#define SELECT V1
#define MUXREG V2
#define PREDS  V3

	{
		R1 = add(R0, #0);               /* R1 = base address of vector regs */
		R6 = #0x00000000;               /* R6 = 0 */
		R7 = ##0x01010101;              /* R7 = select pattern */
	}
	{
		vmem(R1++#1) = V0;              /* Save V0 */
		ZEROS = vsplat(R6);             /* ZEROS = all zeros vector */
		R6 = #1;                        /* R6 = shift amount */
	}
	{
		vmem(R1++#1) = V1;              /* Save V1 */
		SELECT = vsplat(R7);            /* SELECT = 0x01010101... */
	}
	{
		vmem(R1++#1) = V2;              /* Save V2 */
		MUXREG = vmux(Q0, SELECT, ZEROS); /* Extract Q0 predicate */
		SELECT.w = vasl(SELECT.w, R6);  /* SELECT = 0x02020202... */
	}
	{
		vmem(R1++#1) = V3;              /* Save V3 */
		PREDS = vmux(Q1, SELECT, ZEROS); /* Extract Q1 predicate */
		SELECT.w = vasl(SELECT.w, R6);  /* SELECT = 0x04040404... */
	}
	{
		vmem(R1++#1) = V4;              /* Save V4 */
		MUXREG = vmux(Q2, SELECT, ZEROS); /* Extract Q2 predicate */
		PREDS = vor(PREDS, MUXREG);     /* Combine predicates */
		SELECT.w = vasl(SELECT.w, R6);  /* SELECT = 0x08080808... */
	}
	{
		vmem(R1++#1) = V5;              /* Save V5 */
		MUXREG = vmux(Q3, SELECT, ZEROS); /* Extract Q3 predicate */
		PREDS = vor(PREDS, MUXREG);     /* Combine predicates */
		R6 = add(R0, #0);               /* R6 = base address again */
	}
	{
		V0 = vmem(R6++#1);              /* Restore V0 (start undoing damage) */
		vmem(R1++#1) = V6;              /* Save V6 */
		PREDS = vor(PREDS, MUXREG);     /* Final predicate combination */
		R7 = add(R0, #(32*128));        /* R7 = address of vecpredsave */
	}
	{
		V1 = vmem(R6++#1);              /* Restore V1 */
		vmem(R1++#1) = V7;              /* Save V7 */
	}
	{
		V2 = vmem(R6++#1);              /* Restore V2 */
		vmem(R1++#1) = V8;              /* Save V8 */
	}
	{
		V3 = vmem(R6++#1);              /* Restore V3 */
		vmem(R7) = PREDS;               /* Save combined predicates */
	}

#undef ZEROS
#undef SELECT
#undef MUXREG
#undef PREDS

	/* Save remaining vector registers V9-V31 */
	vmem(R1++#1) = V9;
	vmem(R1++#1) = V10;
	vmem(R1++#1) = V11;
	vmem(R1++#1) = V12;
	vmem(R1++#1) = V13;
	vmem(R1++#1) = V14;
	vmem(R1++#1) = V15;
	vmem(R1++#1) = V16;
	vmem(R1++#1) = V17;
	vmem(R1++#1) = V18;
	vmem(R1++#1) = V19;
	vmem(R1++#1) = V20;
	vmem(R1++#1) = V21;
	vmem(R1++#1) = V22;
	vmem(R1++#1) = V23;
	vmem(R1++#1) = V24;
	vmem(R1++#1) = V25;
	vmem(R1++#1) = V26;
	vmem(R1++#1) = V27;
	vmem(R1++#1) = V28;
	vmem(R1++#1) = V29;
	vmem(R1++#1) = V30;
	vmem(R1++#1) = V31;

	jumpr R31

/**
 * @brief Restore HVX vector registers and predicates
 *
 * void hvx_restore_context_asm(struct hvx_vectors *vregs)
 *
 * @param R0 Pointer to hvx_vectors structure
 */
GTEXT(hvx_restore_context_asm)
SECTION_FUNC(TEXT, hvx_restore_context_asm)
	/* Define temporary vector registers for predicate handling */
#define PREDS  V30
#define MASK   V31
#define CMPREG V29

	{
		R6 = add(R0, #(32*128));        /* R6 = address of vecpredsave */
		R8 = ##0x01010101;              /* R8 = mask pattern */
	}
	{
		PREDS = vmem(R6);               /* Load saved predicates */
		MASK = vsplat(R8);              /* MASK = 0x01010101... */
		R7 = #1;                        /* R7 = shift amount */
	}
	{
		CMPREG = vand(PREDS, MASK);     /* Extract Q0 bits */
		V0 = vmem(R0++#1);              /* Load V0 */
	}
	{
		Q0 = vcmp.eq(CMPREG.b, MASK.b); /* Restore Q0 predicate */
		V1 = vmem(R0++#1);              /* Load V1 */
		MASK.w = vasl(MASK.w, R7);      /* MASK = 0x02020202... */
	}
	{
		CMPREG = vand(PREDS, MASK);     /* Extract Q1 bits */
		V2 = vmem(R0++#1);              /* Load V2 */
	}
	{
		Q1 = vcmp.eq(CMPREG.b, MASK.b); /* Restore Q1 predicate */
		V3 = vmem(R0++#1);              /* Load V3 */
		MASK.w = vasl(MASK.w, R7);      /* MASK = 0x04040404... */
	}
	{
		CMPREG = vand(PREDS, MASK);     /* Extract Q2 bits */
		V4 = vmem(R0++#1);              /* Load V4 */
	}
	{
		Q2 = vcmp.eq(CMPREG.b, MASK.b); /* Restore Q2 predicate */
		V5 = vmem(R0++#1);              /* Load V5 */
		MASK.w = vasl(MASK.w, R7);      /* MASK = 0x08080808... */
	}
	{
		CMPREG = vand(PREDS, MASK);     /* Extract Q3 bits */
		V6 = vmem(R0++#1);              /* Load V6 */
	}
	{
		Q3 = vcmp.eq(CMPREG.b, MASK.b); /* Restore Q3 predicate */
		V7 = vmem(R0++#1);              /* Load V7 */
	}

#undef PREDS
#undef MASK
#undef CMPREG

	/* Restore remaining vector registers V8-V31 */
	V8 = vmem(R0++#1);
	V9 = vmem(R0++#1);
	V10 = vmem(R0++#1);
	V11 = vmem(R0++#1);
	V12 = vmem(R0++#1);
	V13 = vmem(R0++#1);
	V14 = vmem(R0++#1);
	V15 = vmem(R0++#1);
	V16 = vmem(R0++#1);
	V17 = vmem(R0++#1);
	V18 = vmem(R0++#1);
	V19 = vmem(R0++#1);
	V20 = vmem(R0++#1);
	V21 = vmem(R0++#1);
	V22 = vmem(R0++#1);
	V23 = vmem(R0++#1);
	V24 = vmem(R0++#1);
	V25 = vmem(R0++#1);
	V26 = vmem(R0++#1);
	V27 = vmem(R0++#1);
	V28 = vmem(R0++#1);
	V29 = vmem(R0++#1);
	V30 = vmem(R0++#1);
	V31 = vmem(R0++#1);

	jumpr R31

/**
 * @brief Set HVX hardware configuration
 *
 * void hvx_set_hardware_config(uint8_t vlength, uint8_t extbits)
 *
 * @param R0 Vector length configuration
 * @param R1 Extension bits configuration
 */
GTEXT(hvx_set_hardware_config)
SECTION_FUNC(TEXT, hvx_set_hardware_config)
	{
		R2 = asl(R1, #8);               /* Shift extbits to upper byte */
	}
	{
		R2 = or(R0, R2);                /* Combine vlength and extbits */
	}
	{
		R3 = #0;                        /* Additional config parameter */
		R4 = #0;                        /* Reserved parameter */
	}
	/* Use VM hardware config instruction to set HVX parameters */
	/* Note: Actual VM instruction may need to be adjusted based on available VM interface */
	trap1(#1);                          /* Placeholder for VM config call */
	jumpr R31

/**
 * @brief Get HVX hardware configuration
 *
 * uint32_t hvx_get_hardware_config(void)
 *
 * @return Current hardware configuration in R0
 */
GTEXT(hvx_get_hardware_config)
SECTION_FUNC(TEXT, hvx_get_hardware_config)
	/* Read current HVX configuration from hardware */
	/* Implementation depends on available system registers */
	{
		R0 = #0;                        /* Placeholder - replace with actual read */
	}
	jumpr R31
