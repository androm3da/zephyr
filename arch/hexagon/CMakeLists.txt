# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(
  core/cpu_idle.S
  core/fatal.c
  core/irq_manage.c
  core/reset.S
  core/swap.S
  core/thread.c
  core/vector_table.S
)

zephyr_library_sources_ifdef(CONFIG_HEXAGON_HAS_V60 core/cache_v60.c)
zephyr_library_sources_ifdef(CONFIG_HEXAGON_HAS_V65 core/cache_v65.c)
zephyr_library_sources_ifdef(CONFIG_HEXAGON_HAS_V66 core/cache_v66.c)

add_subdirectory(core)
