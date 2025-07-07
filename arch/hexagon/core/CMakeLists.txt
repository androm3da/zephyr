# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(
  crt0.S
  hexagon_rt.S
  timer.c
  prep_c.c
  vm_entry.S
  vm_events.c
)
