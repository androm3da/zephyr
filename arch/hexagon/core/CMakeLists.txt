# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

zephyr_library()

zephyr_library_sources(
  prep_c.c
  thread.c
  cpu_idle.c
  fatal.c
)

zephyr_library_sources_ifdef(CONFIG_HEXAGON_VM hvm_event_vectors.S)
zephyr_library_sources_ifdef(CONFIG_HEXAGON_VM vmsetie_cached.c)
zephyr_library_sources_ifndef(CONFIG_HEXAGON_VM bare_event_vectors.S)

zephyr_library_sources_ifdef(CONFIG_IRQ_OFFLOAD irq_offload.c)
zephyr_library_sources_ifdef(CONFIG_SEMIHOST semihost.c)
