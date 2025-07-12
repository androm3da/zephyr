# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

add_subdirectory(core)

# Add hexagon-specific include directory
zephyr_include_directories(include)

set_property(GLOBAL PROPERTY PROPERTY_OUTPUT_FORMAT elf32-littlehexagon)
