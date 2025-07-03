/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>
#include <zephyr/linker/sections.h>

GTEXT(_reset_handler)
GTEXT(__start)

SECTION_FUNC(TEXT, _reset_handler)
    /* Jump to start */
    jump __start
