# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

set(SUPPORTED_EMU_PLATFORMS qemu)
set(EMU_PLATFORM qemu)
set(QEMU_ARCH hexagon)

set(QEMU_binary_suffix hexagon)
set(QEMU_CPU_TYPE_${ARCH} v67)

# Basic QEMU flags for Hexagon emulation
set(QEMU_FLAGS_${ARCH}
  -cpu ${QEMU_CPU_TYPE_${ARCH}}
  -nographic
  -m 4G
  )

if(CONFIG_SEMIHOSTING)
    list(APPEND QEMU_FLAGS_${ARCH} -semihosting)
endif()

# Network support for advanced testing
if(CONFIG_NET_QEMU_ETHERNET)
  set(QEMU_NET_ARGS
    -netdev user,id=net0,hostfwd=tcp::5555-:5555
    -device virtio-net-device,netdev=net0
    )
  list(APPEND QEMU_FLAGS_${ARCH} ${QEMU_NET_ARGS})
endif()

# Debug support with GDB server
set(QEMU_DEBUG_FLAGS
  -gdb tcp::1234
  -S
  )

# Set up debugger and flasher
board_set_debugger_ifnset(qemu)
board_set_flasher_ifnset(qemu)

board_runner_args(qemu "-machine;virt")
board_runner_args(qemu "-gdb-port;1234")

# Finalize runner configuration
board_finalize_runner_args(qemu)
