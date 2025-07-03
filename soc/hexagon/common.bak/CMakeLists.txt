# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_sources(
  soc.c
)

zephyr_linker_script(
  LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
)
