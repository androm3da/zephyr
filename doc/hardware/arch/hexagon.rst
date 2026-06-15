.. _hexagon_developer_guide:

Hexagon Developer Guide
#######################

Overview
********

The Qualcomm Hexagon DSP is a VLIW (Very Long Instruction Word) processor
architecture found in Qualcomm SoCs. Zephyr runs on Hexagon as a guest
operating system under the `Hexagon VM
<https://docs.qualcomm.com/bundle/publicresource/80-NB419-3_REV_A_Hexagin_Virtual_Machine_Specification.pdf>`_,
using the HVM interface for all privileged operations including interrupt
handling, timer management, and memory management.  One `implementation of
the Hexagon VM spec is h2<https://github.com/qualcomm/hexagon-hypervisor>`_.

.. note::

   Zephyr must run as a Hexagon VM guest. Bare-metal hexagon execution
   (e.g., ``-machine virt -kernel zephyr.elf``) is not supported. The
   Hexagon system architecture continues to evolve.  The Hexagon VM provides
   a stable interface that implements interrupt delivery, timer operations,
   and memory management. See :ref:`hexagon_qemu` for the
   correct boot procedure.

The current port targets the ``qemu_hexagon/qemu_hexagon_virt`` board and
supports threads, preemptive scheduling, interrupts, timers, MMU-based memory
protection, userspace, HVX vector extensions, GDB stub, and coredump.

Supported Features
******************

The table below summarizes the status of key OS features in the Hexagon port.

+---------------------------------------------+-------------+
| Feature                                     | Status      |
+=============================================+=============+
| Preemptive multi-threading                  | Y           |
+---------------------------------------------+-------------+
| Cooperative multi-threading                 | Y           |
+---------------------------------------------+-------------+
| Thread local storage (TLS)                  | Y           |
+---------------------------------------------+-------------+
| Dynamic interrupts                          | Y           |
+---------------------------------------------+-------------+
| Direct ISR support                          | Y           |
+---------------------------------------------+-------------+
| IRQ offload (software interrupt)            | Y           |
+---------------------------------------------+-------------+
| CPU idling                                  | Y           |
+---------------------------------------------+-------------+
| System timer                                | Y           |
+---------------------------------------------+-------------+
| MMU-based memory protection                 | Y           |
+---------------------------------------------+-------------+
| Userspace                                   | Y           |
+---------------------------------------------+-------------+
| HW stack protection                         | Y           |
+---------------------------------------------+-------------+
| Stack sentinel                              | Y           |
+---------------------------------------------+-------------+
| HVX vector extensions                       | Y           |
+---------------------------------------------+-------------+
| GDB stub                                    | Y           |
+---------------------------------------------+-------------+
| Coredump                                    | Y           |
+---------------------------------------------+-------------+
| Stack walking                               | Y           |
+---------------------------------------------+-------------+
| Power management                            | Y           |
+---------------------------------------------+-------------+
| picolibc C library                          | Y           |
+---------------------------------------------+-------------+
| SMP                                         | N           |
+---------------------------------------------+-------------+

Hexagon VM Integration
**********************

Zephyr runs as a guest OS under the Hexagon VM. All privileged operations
are performed via ``trap1`` hypercalls to the hypervisor. The key VM operations
used are:

- ``vmsetvec``: Set the Guest Event Vector Base (GEVB)
- ``vmnewmap``: Install a page table
- ``vmsetie``: Set the interrupt enable (IE) flag
- ``vmrte``: Return from event (restores guest execution state)
- ``vmintop``: Interrupt controller operations (enable, disable, post, status)
- ``vmtimerop``: Timer operations (program timeout, query frequency)

Guest Event Vector Base (GEVB)
==============================

The Hexagon VM dispatches events to the guest through the GEVB, a table of
jump instructions at fixed offsets. Each entry is a single 4-byte jump
instruction to the corresponding handler. The entries include Reset, Machine
Check, General Exception, Debug, Trap0 (syscall), and Interrupt.

On event delivery, the Hexagon VM provides the guest return address in register ``G0``
(GELR) and the guest status in ``G1`` (GSR), with interrupts disabled
(``GSR.IE = 0``). The event handlers save context, dispatch to the appropriate
C handler, and return to the interrupted code via ``vmrte``.

Event Entry and Exit
====================

The ``EVENT_ENTRY`` and ``EVENT_EXIT`` macros in
:file:`arch/hexagon/core/event_handlers.S` manage the register save and restore
for all event types:

- **EVENT_ENTRY** allocates an 80-byte event context frame on the interrupted
  code's stack and saves all volatile registers (r0-r15), predicate registers,
  the link register, GELR (return PC), and GSR (guest status).

- **EVENT_EXIT** restores the saved context and checks for pending thread
  preemption. If a context switch is needed, it calls ``z_hexagon_arch_switch``
  before returning to the interrupted code via ``vmrte``.

Only volatile registers (r0-r15) are saved in the event frame. Callee-saved
registers (r16-r27) are preserved by the C calling convention and are
saved/restored during context switching.

Interrupt Handling
******************

The Hexagon VM delivers virtual interrupts to the guest with the interrupt number encoded
in the ``GSR.CAUSE`` field (bits 7:0). There is no hardware interrupt
controller (such as an L2VIC) visible to the guest; all interrupt management
is performed through VM operations.

The interrupt handler in :file:`arch/hexagon/core/irq_manage.c`:

1. Extracts the IRQ number from ``GSR.CAUSE``
2. Increments the ISR nesting counter
3. Dispatches to the registered ISR via the software interrupt table
4. Re-enables the interrupt via ``globen`` (end-of-interrupt acknowledgment)
5. Decrements the ISR nesting counter

Interrupt Controller
====================

The VM-based interrupt controller in :file:`arch/hexagon/core/intc.c` provides
these operations through VM hypercalls:

- **Enable**: ``hexagon_vm_intop_globen(irq)``
- **Disable**: ``hexagon_vm_intop_globdis(irq)``
- **Post (software trigger)**: ``hexagon_vm_intop_post(irq, 0)``
- **Query pending**: ``hexagon_vm_intop_get()``
- **Query status**: ``hexagon_vm_intop_status(irq)``

The architecture supports 64 virtual interrupt lines. CPU interrupts (0-15)
only support ``globen``/``globdis`` operations; ``locen``/``locdis`` return
``-1`` for these.

IRQ offload is supported via software interrupt posting, allowing deferred
interrupt processing from thread context.

Timer
*****

The Hexagon timer driver in :file:`drivers/timer/hexagon_timer.c` uses the
Hexagon VM ``vmtimerop`` interface to program periodic tick interrupts.

- **Timer IRQ**: Virtual interrupt 12 (``HVM_TIME_GUESTINT``), delivered
  directly by the Hexagon VM rather than through an interrupt controller.
- **Programming**: ``hexagon_vm_timerop(deltatimeout, 0, cycles_per_tick)``
  programs a one-shot timeout in the nanosecond frequency domain.
- **Frequency**: Queried at boot via ``hexagon_vm_timerop(getfreq, 0, 0)``.
- **Tick rate**: Configured by :kconfig:option:`CONFIG_SYS_CLOCK_TICKS_PER_SEC`
  (default 1000, i.e., 1 ms tick).

The timer ISR accumulates elapsed cycles and announces ticks to the kernel via
``sys_clock_announce()``.

Memory Management
*****************

The Hexagon port uses a simple page table configured during early boot in
:file:`arch/hexagon/core/hvm_event_vectors.S`. The page table uses 4 MB page
directory entries (PDEs) and is installed via the ``vmnewmap`` hypercall.

The Hexagon VM performs two-stage address translation:

1. **Guest VA to Guest PA**: Via the guest page table
2. **Guest PA to Host PA**: Via the VM's guestmap

For device I/O (e.g., the PL011 UART at ``0x10000000``), the PDE must set the
``__HVM_PTE_SHARED`` bit (bit 3). This tells the VM to bypass guestmap translation
and use the PTE's physical page number directly as the host physical address.

The default page table creates identity mappings for:

- Code and data at ``0xa0000000`` (R/W/X, write-back L2 cacheable)
- PL011 UART at ``0x10000000`` (R/W, device memory, shared)
- Hexagon VM kernel region at ``0x9b800000`` (R/W/X, write-back L2 cacheable)

Page sizes supported by the VM include 4 KB, 16 KB, 64 KB, 256 KB, 1 MB,
4 MB, and 16 MB, configured via :kconfig:option:`CONFIG_MMU_PAGE_SIZE`.

CPU Idle
********

The idle implementation in :file:`arch/hexagon/core/cpu_idle.c` must handle the
constraint that the idle loop calls ``arch_cpu_idle()`` with interrupts disabled
(``IE = 0``) and must return with interrupts enabled (``IE = 1``).

The implementation uses ``hexagon_vm_setie(VM_INT_ENABLE)`` rather than
``hexagon_vm_wait()``. When the IE flag transitions from 0 to 1, the Hexagon VM's
interrupt check dispatches any pending interrupts (such as the
timer), allowing the system to wake from idle.

A self-posting timer workaround ensures the timer ISR fires even on QEMU
configurations where the hardware timer counter does not advance continuously.

Userspace
*********

Userspace support is enabled with :kconfig:option:`CONFIG_USERSPACE`. The
implementation in :file:`arch/hexagon/core/userspace.c` uses the Hexagon VM guest mode
mechanism:

- **Entering user mode**: ``vmrte`` with ``GSR`` bit 31 set transitions to user
  mode. The guest return address (``GELR``), stack pointer (``GOSP``), and
  status register (``GSR``) are configured before the transition.

- **System calls**: User mode code triggers a ``trap0`` instruction, which is
  caught by the Trap0 entry in the GEVB. The syscall ID is passed in register
  ``r6`` with up to 5 arguments in ``r1-r5``.

- **Memory protection**: User threads are restricted to their own stack memory.
  The ``arch_buffer_validate()`` function checks memory access permissions
  based on the thread's stack bounds.

- **Memory domains**: Up to 8 memory partitions per domain are supported.

HVX Vector Extensions
*********************

Hexagon Vector Extensions (HVX) provide SIMD vector processing with 32 vector
registers. The implementation in :file:`arch/hexagon/core/hvx.c` supports both
64-byte and 128-byte vector modes.

Enable HVX with:

- :kconfig:option:`CONFIG_HEXAGON_HVX`: Enable HVX support
- :kconfig:option:`CONFIG_HEXAGON_HVX_128B`: Use 128-byte vectors (default)
  instead of 64-byte

HVX contexts are explicitly managed per-thread. Threads that use HVX must
acquire a context, and vector registers are saved/restored when contexts are
enabled or disabled. Context storage is allocated from a static memory slab.

Toolchain
*********

The Hexagon port requires the **LLVM/Clang** toolchain with Hexagon target
support. GCC is not supported.

Key toolchain requirements:

- **Compiler**: Clang with Hexagon target (``hexagon-unknown-linux-musl``)
- **Linker**: ELD linker (:kconfig:option:`CONFIG_LLVM_USE_ELD`)
- **Runtime library**: compiler-rt (:kconfig:option:`CONFIG_COMPILER_RT_RTLIB`)
- **C library**: picolibc (:kconfig:option:`CONFIG_PICOLIBC`)

.. note::

   The linker flags **must** include ``--no-default-config -nostartfiles`` to
   prevent Clang from injecting picolibc's ``crt0-semihost.o`` and
   ``picolibc.ld``, which conflict with Zephyr's startup code and linker
   script.

Example build command:

.. code-block:: shell

   west build -b qemu_hexagon/qemu_hexagon_virt -d build samples/hello_world -- \
     -DZEPHYR_TOOLCHAIN_VARIANT=host -DTOOLCHAIN_VARIANT_COMPILER=llvm \
     -DLLVM_TOOLCHAIN_PATH=/path/to/clang+llvm-hexagon \
     -DCONFIG_LLVM_USE_LLD=y

.. _hexagon_qemu:

QEMU Emulation
**************

The Hexagon port runs on QEMU using the ``virt`` machine type with the Hexagon
VM bootloader (e.g. ``loadlinux``).

Boot Procedure
==============

1. Build the Zephyr application normally with ``west build``.

2. Convert the ELF to a raw binary:

   .. code-block:: shell

      llvm-objcopy -O binary build/zephyr/zephyr.elf build/zephyr/zephyr.bin

3. Boot via the Hexagon VM:

   .. code-block:: shell

      qemu-system-hexagon -M virt -nographic -m 4G \
        -kernel /path/to/hexagon-hypervisor/linux/loadlinux \
        -device "loader,addr=0xa0000000,file=build/zephyr/zephyr.bin"

The ``loadlinux`` bootloader initializes the Hexagon VM and loads the Zephyr
binary at address ``0xa0000000``, where the GEVB reset vector begins execution.

Emulated Peripherals
====================

+---------------------+-------------------+
| Peripheral          | Address           |
+=====================+===================+
| PL011 UART          | ``0x10000000``    |
+---------------------+-------------------+
| System memory       | ``0x80000000``    |
+---------------------+-------------------+
| Code/data SRAM      | ``0xa0000000``    |
+---------------------+-------------------+

Testing
*******

Hexagon tests can be run using the Zephyr twister test runner with the board
target ``qemu_hexagon/qemu_hexagon_virt``:

.. code-block:: shell

   ./scripts/twister -p qemu_hexagon/qemu_hexagon_virt -T tests/kernel/

Architecture-specific tests are located under :file:`tests/arch/hexagon/`.
