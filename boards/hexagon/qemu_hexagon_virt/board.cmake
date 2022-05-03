# Copyright (c) 2022 Qualcomm Innovation Center, Inc.
# SPDX-License-Identifier: Apache-2.0

set(SUPPORTED_EMU_PLATFORMS qemu)
#set(QEMU_ARCH hexagon)

set(QEMU_CPU_TYPE_${ARCH} generic)

set(QEMU_MACH virt)

set(QEMU_FLAGS_${ARCH}
  -cpu ${QEMU_CPU_TYPE_${ARCH}}
  -nographic
  -machine ${QEMU_MACH}
  )

if(CONFIG_XIP)
  # This should be equivalent to
  #   ... -drive if=pflash,file=build/zephyr/zephyr.bin,format=raw
  # without having to pad the binary file to the FLASH size
  set(QEMU_KERNEL_OPTION
  -kernel ${PROJECT_BINARY_DIR}/${CONFIG_KERNEL_BIN_NAME}.bin
  )
endif()

board_set_debugger_ifnset(qemu)
