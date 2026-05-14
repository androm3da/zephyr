.. _zephyr_rtos_debug_plugins:

Zephyr RTOS Debugging Plugins
#############################

Zephyr provides Python-based debugging plugins for both GDB and LLDB that
enable RTOS-aware inspection of kernel objects. These plugins are
architecture-independent and work with any Zephyr-supported target by
resolving struct layouts from DWARF debug info at runtime.

Overview
********

The debugging plugins consist of three modules:

- :file:`scripts/debug/zephyr_core.py` -- Shared core logic that implements
  kernel object walking, offset resolution, and output formatting.
- :file:`scripts/gdb/zephyr_gdb.py` -- GDB frontend that registers the
  ``zephyr`` command prefix.
- :file:`scripts/lldb/zephyr_lldb.py` -- LLDB frontend that registers
  equivalent ``zephyr`` commands.

Both frontends provide the same set of subcommands and produce identical
output.

Prerequisites
*************

- A Zephyr ELF binary built with debug info (the default for debug builds).
- ``CONFIG_THREAD_MONITOR=y`` for full thread enumeration (strongly
  recommended). Without it, only the current thread, idle thread, and
  ready-queue threads are visible.
- ``CONFIG_THREAD_NAME=y`` for human-readable thread names in output.

Available Commands
******************

All commands are accessed through the ``zephyr`` prefix:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Command
     - Description
   * - ``zephyr threads``
     - List all Zephyr threads with state, priority, stack info, and entry
       point.
   * - ``zephyr current``
     - Show detailed information about the currently executing thread.
   * - ``zephyr readyq``
     - Display the ready queue cache and run queue contents.
   * - ``zephyr mutex <expr>``
     - Inspect a ``k_mutex`` (owner, lock count, waiters).
   * - ``zephyr sem <expr>``
     - Inspect a ``k_sem`` (count, limit, waiters).
   * - ``zephyr waitq <expr>``
     - Show threads waiting on a ``_wait_q_t``.

The ``<expr>`` argument is a debugger expression that evaluates to the
address of the kernel object. This can be a symbol name (e.g., ``my_mutex``)
or an explicit cast (e.g., ``*(struct k_mutex *)0x12345``).

Using with GDB
**************

Loading the Plugin
==================

Source the GDB plugin script after connecting to your target:

.. code-block:: none

   (gdb) source scripts/gdb/zephyr_gdb.py
   Zephyr GDB plugin loaded. Type 'zephyr' for help.

To load the plugin automatically, add the source command to your
:file:`.gdbinit` file:

.. code-block:: none

   source /path/to/zephyr/scripts/gdb/zephyr_gdb.py

Example Session
===============

.. code-block:: none

   (gdb) target remote localhost:1234
   (gdb) source scripts/gdb/zephyr_gdb.py
   Zephyr GDB plugin loaded. Type 'zephyr' for help.
   (gdb) zephyr threads
     Address     Name                  State             Prio  Info
   ----------------------------------------------------------------------------------------------------
   > 0x2000abcd  main                  READY                0  stack: 0x20008000 size=4096  entry: main
     0x2000cdef  idle                  READY               15  stack: 0x2000c000 size=1024  entry: idle

   Total: 2 thread(s), current=0x2000abcd
   (gdb) zephyr current
   Thread 0x2000abcd "main" (CURRENT)
     ============================================================
     State:    READY
     Priority: 0
     Entry:    main
     Stack:    0x20008000 - 0x20009000 (size=4096)
   (gdb) zephyr mutex my_mutex
   k_mutex @ 0x2000ef00
     Owner:      0x2000abcd (main)
     Lock count: 1
     Owner orig prio: 0
     Waiters: 0 waiting thread(s)

Using with LLDB
***************

Loading the Plugin
==================

Import the LLDB plugin script after connecting to your target:

.. code-block:: none

   (lldb) command script import scripts/lldb/zephyr_lldb.py
   Zephyr LLDB plugin loaded. Type 'zephyr' for help.

To load the plugin automatically, add the import command to your
:file:`~/.lldbinit` file:

.. code-block:: none

   command script import /path/to/zephyr/scripts/lldb/zephyr_lldb.py

Example Session
===============

.. code-block:: none

   (lldb) command script import scripts/lldb/zephyr_lldb.py
   Zephyr LLDB plugin loaded. Type 'zephyr' for help.
   (lldb) zephyr threads
     Address     Name                  State             Prio  Info
   ----------------------------------------------------------------------------------------------------
   > 0x2000abcd  main                  READY                0  stack: 0x20008000 size=4096  entry: main
     0x2000cdef  idle                  READY               15  stack: 0x2000c000 size=1024  entry: idle

   Total: 2 thread(s), current=0x2000abcd
   (lldb) zephyr sem my_sem
   k_sem @ 0x2000dd00
     Count: 3
     Limit: 10
     Waiters: 0 waiting thread(s)

Thread State Reference
**********************

The ``State`` field shows a bitmask of the thread's current state flags:

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - State
     - Meaning
   * - READY
     - Thread is runnable (state bitmask is 0).
   * - PENDING
     - Thread is waiting on a kernel object (semaphore, mutex, etc.).
   * - SLEEPING
     - Thread is in a timed sleep (``k_sleep``).
   * - SUSPENDED
     - Thread has been explicitly suspended (``k_thread_suspend``).
   * - DEAD
     - Thread has terminated.
   * - QUEUED
     - Thread is in the ready queue.

Multiple flags may be combined (e.g., ``PENDING|SUSPENDED``).

Architecture Support
********************

The plugins work with any architecture supported by Zephyr, including:

- Hexagon
- ARM (Cortex-M, Cortex-A, Cortex-R)
- RISC-V
- Xtensa
- x86
- ARC
- SPARC

No architecture-specific configuration is needed. The plugins resolve all
struct member offsets from the DWARF debug information in the ELF binary
at runtime.

Troubleshooting
***************

No threads found
   Ensure ``CONFIG_THREAD_MONITOR=y`` is enabled in your application
   configuration. Without it, the plugins can only discover threads that
   are currently running, idle, or in the ready queue.

Thread names show as hex addresses
   Enable ``CONFIG_THREAD_NAME=y`` to assign human-readable names to
   threads.

Cannot evaluate expression
   When using ``zephyr mutex``, ``zephyr sem``, or ``zephyr waitq``, the
   expression must be a valid symbol or cast expression that the debugger
   can resolve. Ensure the symbol is in scope or use an explicit address
   cast.

Plugin fails to import
   Verify that the Python path includes the :file:`scripts/debug/`
   directory. The plugin scripts automatically add this path at import
   time, but if you have moved files out of the Zephyr tree, you may need
   to adjust ``sys.path`` manually.
