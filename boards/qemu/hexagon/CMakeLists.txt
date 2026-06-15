# SPDX-License-Identifier: Apache-2.0
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.

set(SUPPORTED_EMU_PLATFORMS qemu)

set(LINKER_SCRIPT ${BOARD_DIR}/linker.ld)

# Board-specific library
zephyr_library()
zephyr_library_sources(board.c)

# Provide tc_util_user_override.h so tests call sys_poweroff() on completion,
# allowing QEMU to exit cleanly without an external timeout/SIGTERM.
zephyr_include_directories(${BOARD_DIR})
