# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: Apache-2.0

"""
LLDB Python plugin for debugging Zephyr RTOS.

Architecture-independent: all struct layouts are resolved from DWARF debug
info at runtime, so this plugin works on any Zephyr-supported architecture
(Hexagon, ARM, RISC-V, Xtensa, x86, ARC, ...).

Provides commands for inspecting Zephyr kernel objects:
  - zephyr threads    : List all threads with state, priority, and stack info
  - zephyr current    : Show details of the current thread
  - zephyr readyq     : Show the ready queue
  - zephyr mutex EXPR : Inspect a k_mutex
  - zephyr sem EXPR   : Inspect a k_sem
  - zephyr waitq EXPR : Show threads waiting on a wait queue

Usage:
  (lldb) command script import scripts/lldb/zephyr_lldb.py
  (lldb) zephyr threads
"""

import os
import sys

import lldb

# Add the shared debug module to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "debug"))
import zephyr_core


# ---------------------------------------------------------------------------
# LLDB Backend
# ---------------------------------------------------------------------------

class LldbBackend(zephyr_core.Backend):
    """Backend implementation using the LLDB Python API."""

    def __init__(self, target, process):
        self.target = target
        self.process = process

    def read_u32(self, addr):
        err = lldb.SBError()
        val = self.process.ReadUnsignedFromMemory(addr, 4, err)
        return None if err.Fail() else val

    def read_u8(self, addr):
        err = lldb.SBError()
        val = self.process.ReadUnsignedFromMemory(addr, 1, err)
        return None if err.Fail() else val

    def read_ptr(self, addr):
        err = lldb.SBError()
        ptr_size = self.target.GetAddressByteSize()
        val = self.process.ReadUnsignedFromMemory(addr, ptr_size, err)
        return None if err.Fail() else val

    def read_string(self, addr, max_len=64):
        err = lldb.SBError()
        buf = self.process.ReadCStringFromMemory(addr, max_len, err)
        return None if err.Fail() else buf

    def get_symbol_addr(self, name):
        syms = self.target.FindSymbols(name)
        if syms.GetSize() == 0:
            return None
        sym = syms.GetContextAtIndex(0).GetSymbol()
        return sym.GetStartAddress().GetLoadAddress(self.target)

    def get_member_offset(self, type_name, member):
        types = self.target.FindTypes(type_name)
        if types.GetSize() == 0:
            return None
        return _find_field_offset(types.GetTypeAtIndex(0), member, 0)

    def get_type_size(self, type_name):
        types = self.target.FindTypes(type_name)
        if types.GetSize() == 0:
            return None
        return types.GetTypeAtIndex(0).GetByteSize()

    def eval_expr(self, expr):
        val = self.target.EvaluateExpression(f"(void *)&({expr})")
        if not val.IsValid() or val.GetError().Fail():
            return None
        return val.GetValueAsUnsigned()

    def resolve_symbol(self, addr):
        addr_obj = lldb.SBAddress(addr, self.target)
        sym = addr_obj.GetSymbol()
        return sym.GetName() if sym.IsValid() else None

    def is_big_endian(self):
        return self.target.GetByteOrder() == lldb.eByteOrderBig


def _find_field_offset(sbtype, member_name, base_offset):
    """Recursively search *sbtype* for a field named *member_name*.

    Anonymous unions/structs (name is ``None``) are descended into
    automatically, accumulating their offset.
    """
    for i in range(sbtype.GetNumberOfFields()):
        field = sbtype.GetFieldAtIndex(i)
        fname = field.GetName()
        if fname == member_name:
            return base_offset + field.GetOffsetInBytes()
        if fname is None and field.GetType().GetNumberOfFields() > 0:
            result = _find_field_offset(
                field.GetType(), member_name,
                base_offset + field.GetOffsetInBytes())
            if result is not None:
                return result
    return None


# ---------------------------------------------------------------------------
# LLDB command wrappers
# ---------------------------------------------------------------------------

def _output(result, lines):
    """Send a list of lines to the LLDB result object."""
    for line in lines:
        result.AppendMessage(line)


def _make_backend(debugger, result):
    """Create an LldbBackend, or print an error and return None."""
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    if not process.IsValid():
        result.AppendMessage("No process.")
        return None
    return LldbBackend(target, process)


def _cmd_threads(debugger, command, result, internal_dict):
    backend = _make_backend(debugger, result)
    if backend:
        _output(result, zephyr_core.cmd_threads(backend))


def _cmd_current(debugger, command, result, internal_dict):
    backend = _make_backend(debugger, result)
    if backend:
        _output(result, zephyr_core.cmd_current(backend))


def _cmd_readyq(debugger, command, result, internal_dict):
    backend = _make_backend(debugger, result)
    if backend:
        _output(result, zephyr_core.cmd_readyq(backend))


def _cmd_mutex(debugger, command, result, internal_dict):
    backend = _make_backend(debugger, result)
    if backend:
        _output(result, zephyr_core.cmd_mutex(backend, command.strip()))


def _cmd_sem(debugger, command, result, internal_dict):
    backend = _make_backend(debugger, result)
    if backend:
        _output(result, zephyr_core.cmd_sem(backend, command.strip()))


def _cmd_waitq(debugger, command, result, internal_dict):
    backend = _make_backend(debugger, result)
    if backend:
        _output(result, zephyr_core.cmd_waitq(backend, command.strip()))


# ---------------------------------------------------------------------------
# Multiplexer command
# ---------------------------------------------------------------------------

_SUBCMDS = {
    "threads": _cmd_threads,
    "current": _cmd_current,
    "readyq": _cmd_readyq,
    "mutex": _cmd_mutex,
    "sem": _cmd_sem,
    "waitq": _cmd_waitq,
}


def _zephyr_command(debugger, command, result, internal_dict):
    """Zephyr RTOS debugging commands."""
    parts = command.strip().split(None, 1)
    subcmd = parts[0] if parts else ""
    rest = parts[1] if len(parts) > 1 else ""

    if subcmd in _SUBCMDS:
        _SUBCMDS[subcmd](debugger, rest, result, internal_dict)
    else:
        _output(result, zephyr_core.cmd_help())
        if subcmd:
            result.AppendMessage(f"\nUnknown subcommand: '{subcmd}'")


# ---------------------------------------------------------------------------
# Plugin registration
# ---------------------------------------------------------------------------

def __lldb_init_module(debugger, internal_dict):
    """Called by LLDB when the module is imported."""
    debugger.HandleCommand(
        'command script add -f zephyr_lldb._zephyr_command zephyr')
    print("Zephyr LLDB plugin loaded. Type 'zephyr' for help.")
