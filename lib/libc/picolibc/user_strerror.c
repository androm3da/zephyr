/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "picolibc-hooks.h"

/*
 * picolibc's _strerror_r() calls the optional user hook _user_strerror() for
 * error numbers it does not recognise, guarding the call with a
 * "&_user_strerror == NULL" test on the weak declaration. The guard makes the
 * call correct at run time, but the call instruction still has to be
 * relocated, and a weak-undefined symbol relocates to address 0. On targets
 * whose text is linked far from 0 -- Hexagon runs at 0xa0000000 -- that
 * displacement does not fit the architecture's PC-relative call, and the link
 * fails with "relocation R_HEX_B22_PCREL out of range".
 *
 * Supplying the hook gives the relocation a nearby target. Returning NULL is
 * exactly what picolibc documents the default _user_strerror() to do, so
 * strerror() behaviour is unchanged. An application that defines its own
 * _user_strerror() still wins: this definition lives in an archive member
 * that the linker then has no reason to pull in.
 */
char *_user_strerror(int errnum, int internal, int *errptr)
{
	ARG_UNUSED(errnum);
	ARG_UNUSED(internal);
	ARG_UNUSED(errptr);

	return NULL;
}
