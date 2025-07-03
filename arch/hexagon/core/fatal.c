/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

FUNC_NORETURN void z_hexagon_fatal_error(unsigned int reason,
                                         const struct arch_esf *esf)
{
    if (esf != NULL) {
        LOG_ERR("Fatal error: %d, USR: 0x%08x", reason, esf->usr);
        LOG_ERR("PC: 0x%08x, SP: 0x%08x", esf->elr, esf->sp);
    }

    z_fatal_error(reason, esf);
    CODE_UNREACHABLE;
}

FUNC_NORETURN void arch_syscall_oops(void *ssf_ptr)
{
    z_hexagon_fatal_error(K_ERR_KERNEL_OOPS, ssf_ptr);
    CODE_UNREACHABLE;
}

void z_hexagon_fatal_error_handler(void)
{
    /* Handle fatal errors */
    z_hexagon_fatal_error(K_ERR_SPURIOUS_IRQ, NULL);
}
