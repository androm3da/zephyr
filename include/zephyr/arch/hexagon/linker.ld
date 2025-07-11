/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Common parts of the linker scripts for the Hexagon targets.
 */

#include <zephyr/linker/sections.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

/* Memory regions */
#define ROMABLE_REGION FLASH
#define RAMABLE_REGION SRAM

/* Hexagon-specific alignment requirements */
#define HEXAGON_PAGE_ALIGN . = ALIGN(4096);
#define HEXAGON_CACHE_ALIGN . = ALIGN(32);
#define HEXAGON_STACK_ALIGN . = ALIGN(8);

/* Memory barrier macros */
#define MEMORY_BARRIER_BEGIN
#define MEMORY_BARRIER_END

/* Text section definition */
#define TEXT_SECTION() \
	GROUP_START(TEXT) \
	_image_text_start = .; \
	__text_region_start = .; \
	SECTION_PROLOGUE(.reset,,) \
	{ \
		KEEP(*(.reset*)) \
	} \
	SECTION_PROLOGUE(.exception_vectors,,) \
	{ \
		KEEP(*(.exception_vectors*)) \
	} \
	SECTION_PROLOGUE(.text,,) \
	{ \
		_image_text_start = .; \
		*(.text) \
		*(.text.*) \
		*(.gnu.linkonce.t.*) \
	} \
	_image_text_end = .; \
	__text_region_end = .; \
	__text_region_size = __text_region_end - __text_region_start; \
	GROUP_END(TEXT)

/* Data section definition */
#define DATA_SECTION() \
	GROUP_START(DATA) \
	HEXAGON_PAGE_ALIGN \
	__data_region_start = .; \
	SECTION_PROLOGUE(.data,,) \
	{ \
		*(.data) \
		*(.data.*) \
		*(.gnu.linkonce.d.*) \
	} \
	__data_region_end = .; \
	__data_region_size = __data_region_end - __data_region_start; \
	__data_region_load_start = LOADADDR(.data); \
	GROUP_END(DATA)

/* BSS section definition */
#define BSS_SECTION() \
	GROUP_START(BSS) \
	HEXAGON_PAGE_ALIGN \
	SECTION_PROLOGUE(.bss,(NOLOAD),) \
	{ \
		__bss_start = .; \
		*(.bss) \
		*(.bss.*) \
		*(COMMON) \
		. = ALIGN(4); \
		__bss_end = .; \
	} \
	GROUP_END(BSS)

/* Stack sections */
#define STACK_SECTION() \
	GROUP_START(STACKS) \
	HEXAGON_STACK_ALIGN \
	SECTION_PROLOGUE(.kernel_stacks,(NOLOAD),) \
	{ \
		__kernel_stacks_start = .; \
		*(.kernel_stacks*) \
		__kernel_stacks_end = .; \
	} \
	HEXAGON_STACK_ALIGN \
	SECTION_PROLOGUE(.user_stacks,(NOLOAD),) \
	{ \
		__user_stacks_start = .; \
		*(.user_stacks*) \
		__user_stacks_end = .; \
	} \
	GROUP_END(STACKS)

/* Heap section */
#define HEAP_SECTION() \
	GROUP_START(HEAP) \
	HEXAGON_STACK_ALIGN \
	SECTION_PROLOGUE(.heap,(NOLOAD),) \
	{ \
		__heap_start = .; \
		. = . + CONFIG_HEAP_MEM_POOL_SIZE; \
		__heap_end = .; \
	} \
	GROUP_END(HEAP)