.. zephyr:board:: qemu_hexagon

Overview
********

This board configuration uses QEMU to emulate a Qualcomm Hexagon DSP virtual
platform.

The Hexagon DSP is a VLIW digital signal processor designed by Qualcomm and
found in Snapdragon SoCs. This board configuration provides a QEMU-based
environment for developing and testing Zephyr applications targeting the
Hexagon architecture.

Zephyr on Hexagon runs as a guest under the Hexagon VM hypervisor.
The ``zephyr.elf`` binary cannot be booted directly; it must be loaded via
the HVM ``loadlinux`` boot loader.

This configuration provides support for the following devices:

* HVM PIC interrupt controller
* HVM timer
* PL011 UART (console)

Hardware
********

Supported Features
==================

.. zephyr:board-supported-hw::

Devices
=======

System Clock
------------

This board configuration uses a system clock frequency of 1 MHz.

Known Problems or Limitations
=============================

The following known issues apply:

* Some tests that exercise ``CONFIG_USERSPACE`` code paths may crash the
  guest VM at runtime.

Programming and Debugging
*************************

.. zephyr:board-supported-runners::

Building
========

This board requires the LLVM/Clang cross-compiler for Hexagon. Set the
``LLVM_TOOLCHAIN_PATH`` environment variable to point to your Hexagon LLVM
toolchain installation.

Build a Zephyr application for this board using:

.. zephyr-app-commands::
   :zephyr-app: samples/hello_world
   :host-os: unix
   :board: qemu_hexagon/qemu_hexagon_virt
   :goals: build
   :gen-args: -DZEPHYR_TOOLCHAIN_VARIANT=host -DTOOLCHAIN_VARIANT_COMPILER=llvm -DCONFIG_LLVM_USE_LLD=y

Running
=======

Zephyr on Hexagon requires the HVM ``loadlinux`` boot loader. If it is
available, ``west build -t run`` will invoke QEMU automatically.

To run manually, convert the ELF to a raw binary and boot via HVM:

.. code-block:: console

   $ llvm-objcopy -O binary build/zephyr/zephyr.elf build/zephyr/zephyr.bin
   $ qemu-system-hexagon \
       -machine virt -nographic -m 4G \
       -kernel path/to/loadlinux \
       -device "loader,addr=0xa0000000,file=build/zephyr/zephyr.bin"

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.

Debugging
=========

Refer to the detailed overview about :ref:`application_debugging`.

QEMU supports remote debugging via the GDB remote serial protocol. Start
QEMU with ``-s -S`` flags to pause execution and wait for a debugger
connection on port 1234.

Use LLDB to connect to the target. GDB does not support the Hexagon ISA.

.. code-block:: console

   $ lldb build/zephyr/zephyr.elf
   (lldb) gdb-remote 1234
   (lldb) b main
   (lldb) continue

References
**********

* `Qualcomm Hexagon V67 Programmer's Reference Manual <https://docs.qualcomm.com/bundle/publicresource/80-N2040-59_AA_Qualcomm_Hexagon_V67_Programmers_Reference_Manual.pdf>`_
* `Hexagon SDK <https://www.qualcomm.com/developer/software/hexagon-dsp-sdk-and-tools>`_ -- includes ``qemu-system-hexagon``
* `QEMU Hexagon source (hex-next branch) <https://github.com/qualcomm/qemu/tree/hex-next>`_ -- build from source
* `Hexagon Hypervisor <https://github.com/qualcomm/hexagon-hypervisor>`_ -- HVM boot loader for guest VMs
