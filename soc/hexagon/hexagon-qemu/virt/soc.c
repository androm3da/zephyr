/*
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief QEMU Hexagon-V virt machine hardware depended interface
 */

#include <kernel.h>
#include <arch/cpu.h>
#include <sys/util.h>

void sys_arch_reboot(int type)
{
    while (1) 
      __asm__ volatile ("pause(#255)\n");
}
