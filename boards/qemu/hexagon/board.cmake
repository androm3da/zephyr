# SPDX-License-Identifier: Apache-2.0

set(SUPPORTED_EMU_PLATFORMS qemu)

set(QEMU_binary_suffix hexagon)
set(QEMU_CPU_TYPE_${ARCH} v66)

set(QEMU_FLAGS_${ARCH}
  -cpu ${QEMU_CPU_TYPE_${ARCH}}
  -machine virt
  -nographic
  -m 4G
  )

board_set_debugger_ifnset(qemu)
