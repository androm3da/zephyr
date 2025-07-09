# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_SOC_QEMU_HEXAGON_VIRT soc.c)

# Set linker script for Hexagon SoC
set(SOC_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld" CACHE INTERNAL "")
