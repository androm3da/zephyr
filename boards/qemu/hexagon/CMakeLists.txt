# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

set(SUPPORTED_EMU_PLATFORMS qemu)

set(LINKER_SCRIPT ${BOARD_DIR}/linker.ld)

# Board-specific library
zephyr_library()
zephyr_library_sources(board.c)
