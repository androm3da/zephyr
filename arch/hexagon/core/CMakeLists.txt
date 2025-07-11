# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

zephyr_library()

zephyr_library_sources(
  prep_c.c
  thread.c
  cpu_idle.c
  fatal.c
  thread_abort.c
  switch.S
  thread_support.c
  thread_entry.S
  irq_manage.c
  intc.c
  event_handlers.S
)

zephyr_library_sources_ifdef(CONFIG_HEXAGON_ENHANCED_CONTEXT_SWITCH switch_enhanced.S)

zephyr_library_sources_ifdef(CONFIG_HEXAGON_VM hvm_event_vectors.S)
zephyr_library_sources_ifndef(CONFIG_HEXAGON_VM bare_event_vectors.S)

zephyr_library_sources_ifdef(CONFIG_GEN_SW_ISR_TABLE isr_tables.c)
zephyr_library_sources_ifdef(CONFIG_IRQ_OFFLOAD irq_offload.c)
zephyr_library_sources_ifdef(CONFIG_SEMIHOST semihost.c)
