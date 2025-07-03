# SPDX-License-Identifier: Apache-2.0

set(QEMU_binary_suffix hexagon)
set(QEMU_CPU_TYPE_${ARCH} v66)

set(QEMU_FLAGS_${ARCH}
  -cpu ${QEMU_CPU_TYPE_${ARCH}}
  -machine virt
  -nographic
  -m 256
  )

board_set_debugger_ifnset(qemu)
