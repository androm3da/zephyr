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

Getting Started
***************

This section describes how to set up the required tools and dependencies to
build and run Zephyr on the QEMU Hexagon board.

Prerequisites
=============

The following tools are required:

* `west` build tool and Zephyr Python dependencies (see Step 4)
* LLVM/Clang cross-compiler for Hexagon (see below)
* QEMU with Hexagon support (see below)
* Hexagon hypervisor ``loadlinux`` boot loader (see below)

Step 1: Install the Hexagon LLVM Toolchain
==========================================

Download the pre-built LLVM cross-compiler for Hexagon from the
`toolchain_for_hexagon releases page
<https://github.com/quic/toolchain_for_hexagon/releases/tag/v22.1.4_>`_.

The recommended asset for Ubuntu 22.04 x86_64 hosts is:

.. code-block:: console

   clang+llvm-22.1.4-cross-hexagon-unknown-linux-musl.tar.zst

Download and extract it to a directory of your choice, for example
``/opt/hexagon-toolchain``:

.. code-block:: console

   $ wget https://artifacts.codelinaro.org/artifactory/codelinaro-toolchain-for-hexagon/22.1.4_/clang+llvm-22.1.4-cross-hexagon-unknown-linux-musl.tar.zst
   $ mkdir -p /opt/hexagon-toolchain
   $ tar -xf clang+llvm-22.1.4-cross-hexagon-unknown-linux-musl.tar.zst \
         -C /opt/hexagon-toolchain --strip-components=2

.. note::

   The tarball contains an ``x86_64-linux-gnu/`` subdirectory at its top
   level.  ``--strip-components=2`` strips both the archive name prefix and
   that subdirectory so that ``bin/clang`` lands directly under the install
   directory.

Set the ``LLVM_TOOLCHAIN_PATH`` environment variable to the extracted
directory:

.. code-block:: console

   $ export LLVM_TOOLCHAIN_PATH=/opt/hexagon-toolchain

Step 2: Build QEMU with Hexagon Support
========================================

Standard QEMU releases do not include Hexagon support. You must build QEMU
from the Qualcomm fork:

.. code-block:: console

   $ git clone https://github.com/qualcomm/qemu.git -b hex-next qemu-hexagon
   $ cd qemu-hexagon
   $ mkdir build && cd build
   $ ../configure --target-list=hexagon-softmmu --disable-werror
   $ make -j$(nproc)

The resulting binary is ``qemu-system-hexagon`` inside the ``build``
directory you are currently in. Add it to your ``PATH`` so that
``west build -t run`` can find it:

.. code-block:: console

   $ export PATH="$PWD:$PATH"

Step 3: Build the Hexagon Hypervisor Boot Loader
=================================================

Zephyr must run as a guest under the Hexagon VM (H2) hypervisor. The
``loadlinux`` binary from the hypervisor repository is used as the QEMU
``-kernel`` argument; it then loads and boots the raw Zephyr binary.

Clone the hypervisor repository. Cloning it directly into ``$ZEPHYR_BASE``
lets the Zephyr build system find ``loadlinux`` automatically:

.. code-block:: console

   $ git clone https://github.com/qualcomm/hexagon-hypervisor.git \
         $ZEPHYR_BASE/hexagon-hypervisor

Build ``loadlinux`` using CMake. The hypervisor uses the same LLVM toolchain
installed in Step 1; override the compiler paths from ``toolchain.cmake`` by
passing them on the CMake command line:

.. code-block:: console

   $ cd $ZEPHYR_BASE/hexagon-hypervisor
   $ mkdir build && cd build
   $ cmake .. \
       -DCMAKE_C_COMPILER=$LLVM_TOOLCHAIN_PATH/bin/clang \
       -DCMAKE_OBJCOPY=$LLVM_TOOLCHAIN_PATH/bin/llvm-objcopy \
       -DCMAKE_RANLIB=$LLVM_TOOLCHAIN_PATH/bin/llvm-ranlib \
       -DCMAKE_NM=$LLVM_TOOLCHAIN_PATH/bin/llvm-nm \
       -DHEXAGON_SIMULATOR=$LLVM_TOOLCHAIN_PATH/bin/hexagon-sim
   $ make -j$(nproc) bootvm_loadlinux

The resulting boot loader is at ``linux/loadlinux`` inside the build
directory. Copy or symlink it to the location the Zephyr build system
expects:

.. code-block:: console

   $ cp linux/loadlinux $ZEPHYR_BASE/hexagon-hypervisor/linux/loadlinux

The build system looks for the boot loader at
``$ZEPHYR_BASE/hexagon-hypervisor/linux/loadlinux`` by default. You can
override this path by setting the ``HEXAGON_H2_LOADLINUX`` environment
variable or passing ``-DHEXAGON_H2_LOADLINUX=<path>`` to CMake.

Step 4: Set Up the Zephyr Environment
======================================

Follow the standard :ref:`getting_started` guide to install Zephyr
dependencies and initialize the workspace.

Create a Python 3.12 virtual environment, install ``west`` and the Zephyr
Python requirements into it, then activate it:

.. code-block:: console

   $ python3.12 -m venv ~/zephyr-venv
   $ ~/zephyr-venv/bin/pip install west
   $ ~/zephyr-venv/bin/pip install -r $ZEPHYR_BASE/scripts/requirements.txt
   $ source ~/zephyr-venv/bin/activate

Source the Zephyr environment script:

.. code-block:: console

   $ source $ZEPHYR_BASE/zephyr-env.sh

Programming and Debugging
*************************

.. zephyr:board-supported-runners::

Building
========

Build a Zephyr application for this board. The toolchain must be specified
explicitly because Hexagon uses a host-variant LLVM cross-compiler:

.. zephyr-app-commands::
   :zephyr-app: samples/hello_world
   :host-os: unix
   :board: qemu_hexagon/qemu_hexagon_virt
   :goals: build
   :gen-args: -DZEPHYR_TOOLCHAIN_VARIANT=host -DTOOLCHAIN_VARIANT_COMPILER=llvm -DLLVM_TOOLCHAIN_PATH=/opt/hexagon-toolchain -DCONFIG_LLVM_USE_LLD=y

.. note::

   Replace ``/opt/hexagon-toolchain`` with the actual value of
   ``$LLVM_TOOLCHAIN_PATH`` set in Step 1.

The build system automatically generates ``zephyr.bin`` (a raw binary) in
addition to ``zephyr.elf``. Both are needed: ``zephyr.elf`` is used for
debugging, and ``zephyr.bin`` is loaded by the hypervisor at runtime.

Running
=======

If ``loadlinux`` is present at the default path and ``qemu-system-hexagon``
is on your ``PATH``, you can run with:

.. code-block:: console

   $ west build -t run

To run manually, pass the path to your QEMU binary and the ``loadlinux``
boot loader explicitly:

.. code-block:: console

   $ qemu-system-hexagon \
       -machine virt -nographic -m 4G \
       -kernel path/to/hexagon-hypervisor/linux/loadlinux \
       -device "loader,addr=0xa0000000,file=build/zephyr/zephyr.bin"

Replace ``path/to/hexagon-hypervisor/linux/loadlinux`` with the actual path
to the ``loadlinux`` binary obtained in Step 3.

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.

Expected output for ``samples/hello_world``:

.. code-block:: console

   *** Booting Zephyr OS build v4.x.x ***
   Hello World! qemu_hexagon/qemu_hexagon_virt

Debugging
=========

Refer to the detailed overview about :ref:`application_debugging`.

QEMU supports remote debugging via the GDB remote serial protocol. Start
QEMU with ``-s -S`` flags to pause execution and wait for a debugger
connection on port 1234:

.. code-block:: console

   $ qemu-system-hexagon \
       -machine virt -nographic -m 4G \
       -kernel path/to/hexagon-hypervisor/linux/loadlinux \
       -device "loader,addr=0xa0000000,file=build/zephyr/zephyr.bin" \
       -s -S

Use LLDB to connect to the target. GDB does not support the Hexagon ISA:

.. code-block:: console

   $ lldb build/zephyr/zephyr.elf
   (lldb) gdb-remote 1234
   (lldb) b main
   (lldb) continue

References
**********

* `Qualcomm Hexagon V67 Programmer's Reference Manual <https://docs.qualcomm.com/bundle/publicresource/80-N2040-59_AA_Qualcomm_Hexagon_V67_Programmers_Reference_Manual.pdf>`_
* `Hexagon toolchain releases (LLVM cross-compiler) <https://github.com/quic/toolchain_for_hexagon/releases>`_
* `QEMU Hexagon source (hex-next branch) <https://github.com/qualcomm/qemu/tree/hex-next>`_
* `Hexagon Hypervisor <https://github.com/qualcomm/hexagon-hypervisor>`_
