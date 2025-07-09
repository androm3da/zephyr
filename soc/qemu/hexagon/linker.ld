#include <zephyr/devicetree.h>
#include <zephyr/linker/sections.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

#define ROMABLE_REGION              RAM
#define RAMABLE_REGION              RAM

/* Memory configuration - simplified for Hexagon */
#define ROM_BASE CONFIG_SRAM_BASE_ADDRESS
#define ROM_SIZE (CONFIG_SRAM_SIZE * 1024)

#define RAM_BASE CONFIG_SRAM_BASE_ADDRESS
#define RAM_SIZE (CONFIG_SRAM_SIZE * 1024)

MEMORY
{
    RAM (rwx)     : ORIGIN = RAM_BASE,    LENGTH = RAM_SIZE
    IDT_LIST (wx) : ORIGIN = 0xFFFFF7FF, LENGTH = 2K
}

ENTRY(__start)

SECTIONS
{
    #include <zephyr/linker/rel-sections.ld>

    SECTION_PROLOGUE(.plt,,)
    {
        *(.plt)
    }

    SECTION_PROLOGUE(.iplt,,)
    {
        *(.iplt)
    }

    GROUP_START(ROMABLE_REGION)
    __rom_region_start = .;

    /* Reset vector must be at the beginning */
    SECTION_PROLOGUE(.reset.__start,,)
    {
        . = ALIGN(4);
        KEEP(*(.reset.__start))
    } GROUP_LINK_IN(ROMABLE_REGION)

    /* Event vector table */
    SECTION_PROLOGUE(.event_vectors._event_vector_table,,)
    {
        . = ALIGN(4);
        KEEP(*(.event_vectors._event_vector_table))
    } GROUP_LINK_IN(ROMABLE_REGION)

    SECTION_PROLOGUE(_TEXT_SECTION_NAME,,)
    {
        . = ALIGN(4);
        __text_region_start = .;
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
        #include <zephyr/linker/kobject-text.ld>
    } GROUP_LINK_IN(ROMABLE_REGION)

    __text_region_end = .;
    __rodata_region_start = .;

    #include <zephyr/linker/common-rom.ld>
    #include <zephyr/linker/thread-local-storage.ld>

    SECTION_PROLOGUE(_RODATA_SECTION_NAME,,)
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        #include <zephyr/linker/kobject-rom.ld>
    } GROUP_LINK_IN(ROMABLE_REGION)

    #include <zephyr/linker/cplusplus-rom.ld>

    __rodata_region_end = .;
    __rom_region_end = .;

    GROUP_END(ROMABLE_REGION)

    GROUP_START(RAMABLE_REGION)

    SECTION_DATA_PROLOGUE(_DATA_SECTION_NAME,,)
    {
        . = ALIGN(4);
        _image_ram_start = .;
        __data_region_start = .;
        __data_start = .;

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.s.*)

        __data_end = .;
    } GROUP_DATA_LINK_IN(RAMABLE_REGION, ROMABLE_REGION)

    __data_size = __data_end - __data_start;
    __data_load_start = LOADADDR(_DATA_SECTION_NAME);

    #include <zephyr/linker/common-ram.ld>
    #include <zephyr/linker/cplusplus-ram.ld>

    __data_region_end = .;

    SECTION_DATA_PROLOGUE(_BSS_SECTION_NAME,(NOLOAD),)
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)

    SECTION_PROLOGUE(_NOINIT_SECTION_NAME,(NOLOAD),)
    {
        *(.noinit .noinit.*)
    } GROUP_LINK_IN(RAMABLE_REGION)

    /* Interrupt stack section */
    SECTION_PROLOGUE(.interrupt_stack,(NOLOAD),)
    {
        . = ALIGN(8);
        _interrupt_stack = .;
        . = . + CONFIG_ISR_STACK_SIZE;
        _interrupt_stack_end = .;
    } GROUP_LINK_IN(RAMABLE_REGION)

    #include <zephyr/linker/ram-end.ld>

    GROUP_END(RAMABLE_REGION)

    #ifdef CONFIG_GEN_ISR_TABLES
    #include <zephyr/linker/intlist.ld>
    #endif

    #include <zephyr/linker/debug-sections.ld>
    /DISCARD/ : { *(.note.GNU-stack) }

    /* Final section for deterministic sizing */
    SECTION_PROLOGUE(.last_section,,)
    {
        _end = .;
        _image_end = .;
    } GROUP_LINK_IN(RAMABLE_REGION)
}
