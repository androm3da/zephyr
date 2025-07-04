/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>

/* Hexagon ABI runtime support functions */

GTEXT(__restore_r16_through_r17_and_deallocframe)
GTEXT(__restore_r16_through_r19_and_deallocframe)
GTEXT(__restore_r16_through_r21_and_deallocframe)
GTEXT(__restore_r16_through_r23_and_deallocframe)
GTEXT(__restore_r16_through_r27_and_deallocframe)
GTEXT(__restore_r16_through_r17_and_deallocframe_before_tailcall)
GTEXT(__restore_r16_through_r19_and_deallocframe_before_tailcall)
GTEXT(__hexagon_divsi3)
GTEXT(__hexagon_udivdi3)

/* Simple stub implementations for now */
SECTION_FUNC(TEXT, __restore_r16_through_r17_and_deallocframe)
    r16 = memw(r29+#0)
    r17 = memw(r29+#4)
    r29 = add(r29, #8)
    jumpr r31

SECTION_FUNC(TEXT, __restore_r16_through_r19_and_deallocframe)
    r16 = memw(r29+#0)
    r17 = memw(r29+#4)
    r18 = memw(r29+#8)
    r19 = memw(r29+#12)
    r29 = add(r29, #16)
    jumpr r31

SECTION_FUNC(TEXT, __restore_r16_through_r21_and_deallocframe)
    r16 = memw(r29+#0)
    r17 = memw(r29+#4)
    r18 = memw(r29+#8)
    r19 = memw(r29+#12)
    r20 = memw(r29+#16)
    r21 = memw(r29+#20)
    r29 = add(r29, #24)
    jumpr r31

SECTION_FUNC(TEXT, __restore_r16_through_r23_and_deallocframe)
    r16 = memw(r29+#0)
    r17 = memw(r29+#4)
    r18 = memw(r29+#8)
    r19 = memw(r29+#12)
    r20 = memw(r29+#16)
    r21 = memw(r29+#20)
    r22 = memw(r29+#24)
    r23 = memw(r29+#28)
    r29 = add(r29, #32)
    jumpr r31

SECTION_FUNC(TEXT, __restore_r16_through_r27_and_deallocframe)
    r16 = memw(r29+#0)
    r17 = memw(r29+#4)
    r18 = memw(r29+#8)
    r19 = memw(r29+#12)
    r20 = memw(r29+#16)
    r21 = memw(r29+#20)
    r22 = memw(r29+#24)
    r23 = memw(r29+#28)
    r24 = memw(r29+#32)
    r25 = memw(r29+#36)
    r26 = memw(r29+#40)
    r27 = memw(r29+#44)
    r29 = add(r29, #48)
    jumpr r31

SECTION_FUNC(TEXT, __restore_r16_through_r17_and_deallocframe_before_tailcall)
    r16 = memw(r29+#0)
    r17 = memw(r29+#4)
    r29 = add(r29, #8)
    jumpr r31

SECTION_FUNC(TEXT, __restore_r16_through_r19_and_deallocframe_before_tailcall)
    r16 = memw(r29+#0)
    r17 = memw(r29+#4)
    r18 = memw(r29+#8)
    r19 = memw(r29+#12)
    r29 = add(r29, #16)
    jumpr r31

/* Simple integer division */
SECTION_FUNC(TEXT, __hexagon_divsi3)
    /* r0 = dividend, r1 = divisor, return quotient in r0 */
    /* Simple loop-based division for now */
    p0 = cmp.eq(r1, #0)
    if (p0) jump div_by_zero
    
    r2 = #0      /* quotient */
    r3 = r0      /* remainder */
    
div_loop:
    p0 = cmp.gtu(r1, r3)
    if (p0) jump div_done
    r3 = sub(r3, r1)
    r2 = add(r2, #1)
    jump div_loop
    
div_done:
    r0 = r2
    jumpr r31
    
div_by_zero:
    r0 = #0
    jumpr r31

/* 64-bit unsigned division */
SECTION_FUNC(TEXT, __hexagon_udivdi3)
    /* r1:0 = dividend, r3:2 = divisor, return quotient in r1:0 */
    /* Simple implementation - just return lower 32-bit division for now */
    p0 = cmp.eq(r2, #0)
    if (p0) jump udiv_by_zero
    
    /* Simple case: if high parts are zero, do 32-bit division */
    p0 = cmp.eq(r1, #0)
    p1 = cmp.eq(r3, #0)
    p0 = and(p0, p1)
    if (p0) jump udiv_32bit
    
    /* For 64-bit case, return 0 for now */
    r1 = #0
    r0 = #0
    jumpr r31
    
udiv_32bit:
    /* Use existing 32-bit division */
    call __hexagon_divsi3
    r1 = #0
    jumpr r31
    
udiv_by_zero:
    r1 = #0
    r0 = #0
    jumpr r31