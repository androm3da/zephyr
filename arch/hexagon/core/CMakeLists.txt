# SPDX-License-Identifier: Apache-2.0
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

zephyr_library()

zephyr_library_sources(
  hvm_event_vectors.S
  event_handlers.S
  switch.S
  prep_c.c
  thread.c
  cpu_idle.c
  fatal.c
  irq_manage.c
  intc.c
  mem_manage.c
)

zephyr_library_sources_ifdef(CONFIG_THREAD_LOCAL_STORAGE tls.c)

zephyr_library_sources_ifdef(CONFIG_HW_STACK_PROTECTION stack_protect.c)
zephyr_library_sources_ifdef(CONFIG_PM power.c)
zephyr_library_sources_ifdef(CONFIG_GDBSTUB gdbstub.c)
zephyr_library_sources_ifdef(CONFIG_DEBUG_COREDUMP coredump.c)
zephyr_library_sources_ifdef(CONFIG_HEXAGON_HVX hvx.c)
zephyr_library_sources_ifdef(CONFIG_HEXAGON_HVX hvx_context.S)

# Add HVX compiler flags when HVX is enabled
if(CONFIG_HEXAGON_HVX)
  # Add HVX support flags
  zephyr_compile_options(-mhvx)

  # Set vector length based on configuration
  if(CONFIG_HEXAGON_HVX_128B)
    zephyr_compile_options(-mhvx-length=128b)
  else()
    zephyr_compile_options(-mhvx-length=64b)
  endif()
endif()
