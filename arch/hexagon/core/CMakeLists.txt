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
  thread_entry.S
  irq_manage.c
  intc.c
  event_handlers.S
)


zephyr_library_sources_ifdef(CONFIG_HEXAGON_VM hvm_event_vectors.S)
zephyr_library_sources_ifdef(CONFIG_HEXAGON_VM vmsetie_cached.c)
zephyr_library_sources_ifndef(CONFIG_HEXAGON_VM bare_event_vectors.S)

zephyr_library_sources_ifdef(CONFIG_IRQ_OFFLOAD irq_offload.c)
zephyr_library_sources_ifdef(CONFIG_SEMIHOST semihost.c)

zephyr_library_sources(mem_manage.c)

# Atomic operations are header-only, bit operations are built-in

# Enable TLS support - enhanced thread structure is now integrated
zephyr_library_sources_ifdef(CONFIG_THREAD_LOCAL_STORAGE tls.c)
# zephyr_library_sources_ifdef(CONFIG_HEXAGON_HW_THREADS hw_threads.c)
