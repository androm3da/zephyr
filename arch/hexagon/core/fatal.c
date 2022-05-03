/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <sys/printk.h>
#include <logging/log.h>
#include <hexagon/hexagonregs.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

FUNC_NORETURN void z_hexagon_fatal_error(unsigned int reason, const z_arch_esf_t *esf)
{
	if (esf != NULL) {
		printk("...\n");

		printk("ELR   : %08lx\n", esf->elr);

		printk("Status: %08lx\n", esf->ssr);
		printk("BadVA0: %08lx\n", esf->badva0);
		printk("BadVA1: %08lx\n", esf->badva1);
	}

	z_fatal_error(reason, esf);
	CODE_UNREACHABLE;
}

static char *cause_str(ulong_t cause)
{
	switch (cause) {
	case 0:
		return "FIXME";
	default:
		return "unknown";
	}
}

void _Fault(z_arch_esf_t *esf)
{
	ulong_t cause;

	cause = (read_ssr() & SSR_CAUSE_MASK);

	LOG_ERR("");
	LOG_ERR(" cause: 0x%08xld, %s", cause, cause_str(cause));

	z_hexagon_fatal_error(K_ERR_CPU_EXCEPTION, esf);
}
