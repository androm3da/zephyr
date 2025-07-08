/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/devicetree.h>
#include <zephyr/linker/sections.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

/* QEMU Hexagon Virtual Machine Memory Layout */
MEMORY
{
    FLASH (rx)  : ORIGIN = DT_REG_ADDR(DT_CHOSEN(zephyr_flash)), LENGTH = DT_REG_SIZE(DT_CHOSEN(zephyr_flash))
    SRAM (rwx)  : ORIGIN = DT_REG_ADDR(DT_CHOSEN(zephyr_sram)), LENGTH = DT_REG_SIZE(DT_CHOSEN(zephyr_sram))
}

ENTRY(__start)

SECTIONS
{
    GROUP_START(FLASH)

    SECTION_PROLOGUE(.text)
    {
        __text_start = .;
        *(.text.vector_table)
        *(.text.__start)
        *(.text*)
        *(.gnu.linkonce.t.*)
        __text_end = .;
    } GROUP_LINK_IN(FLASH)

    SECTION_PROLOGUE(.rodata)
    {
        __rodata_start = .;
        *(.rodata*)
        *(.gnu.linkonce.r.*)
        __rodata_end = .;
    } GROUP_LINK_IN(FLASH)

    GROUP_END(FLASH)

    GROUP_START(SRAM)

    SECTION_DATA_PROLOGUE(_DATA_SECTION_NAME)
    {
        __data_start = .;
        *(.data*)
        *(.gnu.linkonce.d.*)
        __data_end = .;
    } GROUP_DATA_LINK_IN(SRAM, FLASH)

    SECTION_PROLOGUE(.bss (NOLOAD))
    {
        __bss_start = .;
        *(.bss*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    } GROUP_LINK_IN(SRAM)

    GROUP_END(SRAM)

    /* Ensure linker-generated symbols are defined */
    __image_start = ADDR(.text);
    __image_end = .;
    __kernel_ram_start = ADDR(_DATA_SECTION_NAME);
    __kernel_ram_end = .;
    __kernel_ram_size = __kernel_ram_end - __kernel_ram_start;
}
