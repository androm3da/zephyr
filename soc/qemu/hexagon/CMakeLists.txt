# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_SOC_QEMU_HEXAGON_VIRT soc.c)
