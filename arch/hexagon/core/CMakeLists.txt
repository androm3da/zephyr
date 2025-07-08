# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

zephyr_library()

zephyr_library_sources(
  prep_c.c
  thread.c
  cpu_idle.c
  fatal.c
)

zephyr_library_sources_ifdef(CONFIG_GEN_SW_ISR_TABLE isr_tables.c)
zephyr_library_sources_ifdef(CONFIG_IRQ_OFFLOAD irq_offload.c)

add_subdirectory(offsets)
