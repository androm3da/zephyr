/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/linker/sections.h>
#include <zephyr/devicetree.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

/* Define memory regions from devicetree or defaults */
#ifdef CONFIG_XIP
  #define ROM_ADDR DT_REG_ADDR(DT_CHOSEN(zephyr_flash))
  #define ROM_SIZE DT_REG_SIZE(DT_CHOSEN(zephyr_flash))
#else
  #define ROM_ADDR DT_REG_ADDR(DT_CHOSEN(zephyr_flash))
  #define ROM_SIZE DT_REG_SIZE(DT_CHOSEN(zephyr_flash))
#endif

#define RAM_ADDR DT_REG_ADDR(DT_CHOSEN(zephyr_sram))
#define RAM_SIZE DT_REG_SIZE(DT_CHOSEN(zephyr_sram))

MEMORY
{
    ROMABLE_REGION (rx) : ORIGIN = ROM_ADDR, LENGTH = ROM_SIZE
    RAMABLE_REGION (wx) : ORIGIN = RAM_ADDR, LENGTH = RAM_SIZE - 2K
    /* IDT_LIST: dedicated section for interrupt tables */
    IDT_LIST (wx) : ORIGIN = RAM_ADDR + RAM_SIZE - 2K, LENGTH = 2K
}

ENTRY(__start)

SECTIONS
{
    #include <zephyr/linker/rel-sections.ld>

    /*
     * .plt and .iplt are here according to
     * 'hexagon-unknown-elf-ld --verbose', before text section.
     */
    /DISCARD/ :
    {
        *(.plt)
        *(.iplt)
    }

    GROUP_START(ROMABLE_REGION)

    _image_rom_start = ROM_ADDR;

    SECTION_PROLOGUE(_TEXT_SECTION_NAME,,)
    {
        __text_start = .;
        *(.text.unlikely .text.unlikely.*)
        *(.text._start)
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
        *(.eh_frame)
    } GROUP_LINK_IN(ROMABLE_REGION)

    __text_end = .;

    _image_text_start = __text_start;
    _image_text_end = __text_end;

    __rodata_start = .;

    #include <zephyr/linker/common-rom.ld>
    #include <zephyr/linker/thread-local-storage.ld>

    SECTION_PROLOGUE(_RODATA_SECTION_NAME,,)
    {
        . = ALIGN(4);
        *(.srodata)
        *(.srodata.*)
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)

        /* Hexagon-specific: small data sections */
        . = ALIGN(8);
        PROVIDE(__sdata2_start = .);
        *(.sdata2)
        *(.sdata2.*)
        *(.gnu.linkonce.s2.*)
        PROVIDE(__sdata2_end = .);
    } GROUP_LINK_IN(ROMABLE_REGION)

    #include <zephyr/linker/cplusplus-rom.ld>

    __rodata_end = .;
    _image_rom_end = .;
    __data_rom_start = ALIGN(4);

    GROUP_END(ROMABLE_REGION)

    GROUP_START(RAMABLE_REGION)

    SECTION_DATA_PROLOGUE(_DATA_SECTION_NAME,,)
    {
        __data_start = .;
        __data_ram_start = .;
        __kernel_ram_start = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)

        /* Hexagon small data */
        . = ALIGN(8);
        PROVIDE(__sdata_start = .);
        *(.sdata)
        *(.sdata.*)
        *(.gnu.linkonce.s.*)
        PROVIDE(__sdata_end = .);
    } GROUP_DATA_LINK_IN(RAMABLE_REGION, ROMABLE_REGION)

    __data_end = .;

    #include <zephyr/linker/common-ram.ld>
    #include <zephyr/linker/cplusplus-ram.ld>

    SECTION_DATA_PROLOGUE(_BSS_SECTION_NAME,(NOLOAD),)
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        COMMON_SYMBOLS
        . = ALIGN(4);
        __bss_end = .;
    } GROUP_LINK_IN(RAMABLE_REGION)

    /* Interrupt stack */
    SECTION_PROLOGUE(.irq_stack,(NOLOAD),)
    {
        . = ALIGN(8);
        _interrupt_stack = .;
        . += __z_interrupt_stack_SIZEOF;
    } GROUP_LINK_IN(RAMABLE_REGION)

    __kernel_ram_end = .;
    __kernel_ram_size = __kernel_ram_end - __kernel_ram_start;

    GROUP_END(RAMABLE_REGION)

    /* Interrupt descriptor table */
    GROUP_START(IDT_LIST)

    SECTION_PROLOGUE(idt,(NOLOAD),)
    {
        __idt_start = .;
        KEEP(*(.idt))
        __idt_end = .;
    } GROUP_LINK_IN(IDT_LIST)

    GROUP_END(IDT_LIST)

    _image_ram_start = __data_ram_start;
    _image_ram_end = __bss_end;
    _end = .;

    #include <zephyr/linker/debug-sections.ld>

    /DISCARD/ : { *(.note.GNU-stack) }

    SECTION_PROLOGUE(.hexagon.attributes, 0,)
    {
        KEEP(*(.hexagon.attributes))
        KEEP(*(.gnu.attributes))
    }
}
