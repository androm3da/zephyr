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
    /* Reset vector - must be first in FLASH */
    .reset : {
        KEEP(*(.reset*))
    } > FLASH

    /* Event vectors - must be 32-bit aligned */
    .event_vectors : ALIGN(4) {
        KEEP(*(.event_vectors*))
    } > FLASH

    GROUP_START(FLASH)

    _image_text_start = .;

    SECTION_PROLOGUE(.text)
    {
        __text_start = .;
        *(.text.__start)
        *(.text*)
        *(.gnu.linkonce.t.*)
        __text_end = .;
    } GROUP_LINK_IN(FLASH)

    _image_text_end = .;

    SECTION_PROLOGUE(.rodata)
    {
        __rodata_start = .;
        *(.rodata*)
        *(.gnu.linkonce.r.*)
        __rodata_end = .;
    } GROUP_LINK_IN(FLASH)

    _image_rom_start = .;

    GROUP_END(FLASH)

    GROUP_START(SRAM)

    _image_ram_start = .;

    /* Initialized data section */
    SECTION_DATA_PROLOGUE(_DATA_SECTION_NAME)
    {
        __data_region_start = .;
        __data_start = .;
        *(.data*)
        *(.gnu.linkonce.d.*)
        __data_end = .;
        __data_region_end = .;
    } GROUP_DATA_LINK_IN(SRAM, FLASH)

    __data_region_load_start = LOADADDR(_DATA_SECTION_NAME);

    SECTION_PROLOGUE(.bss (NOLOAD))
    {
        __bss_start = .;
        *(.bss*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    } GROUP_LINK_IN(SRAM)

    _image_ram_end = .;

    /* Interrupt stack */
    .interrupt_stack (NOLOAD) : ALIGN(8) {
        _interrupt_stack = .;
        . = . + CONFIG_ISR_STACK_SIZE;
        _interrupt_stack_end = .;
    } > SRAM

    GROUP_END(SRAM)

    /* Ensure linker-generated symbols are defined */
    __image_start = ADDR(.reset);
    __image_end = .;
    __kernel_ram_start = ADDR(_DATA_SECTION_NAME);
    __kernel_ram_end = .;
    __kernel_ram_size = __kernel_ram_end - __kernel_ram_start;

    _end = .;
}
