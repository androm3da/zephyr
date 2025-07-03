# SPDX-License-Identifier: Apache-2.0

if(CONFIG_UART_PL011)
  zephyr_library()
  zephyr_library_sources(pl011_init.c)
endif()
