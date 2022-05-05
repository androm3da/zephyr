
/*
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief QEMU Hexagon-V virt machine hardware depended interface
 */

#include <kernel.h>
#include <init.h>
#include <soc.h>
#include <arch/cpu.h>
#include <sys/util.h>


#if 1
void sys_arch_reboot(int type)
{
    while (1)
            __asm__ volatile ("pause(#255)\n");
}

/**
 * @brief Perform basic hardware initialization at boot.
 *
 * This needs to be run from the very beginning.
 * So the init priority has to be 0 (zero).
 *
 * @return 0
 */
int hexagon_sys_init(const struct device *arg)
{
    uint32_t key;

    ARG_UNUSED(arg);

    return 0;
}
SYS_INIT(hexagon_sys_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif
