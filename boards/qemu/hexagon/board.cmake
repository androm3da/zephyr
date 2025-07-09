# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

set(SUPPORTED_EMU_PLATFORMS qemu)

set(QEMU_binary_suffix hexagon)
set(QEMU_CPU_TYPE_${ARCH} hexagon)

set(QEMU_FLAGS_${ARCH}
  -machine virt
  -nographic
  )

board_set_debugger_ifnset(qemu)

board_runner_args(qemu "--machine;virt")
