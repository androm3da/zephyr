# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: Apache-2.0

import os
import subprocess
import sys
import logging
import shlex
import re
import pytest
from twister_harness import DeviceAdapter

ZEPHYR_BASE = os.getenv("ZEPHYR_BASE")
sys.path.insert(0, os.path.join(ZEPHYR_BASE, "scripts", "pylib", "twister"))
from twisterlib.cmakecache import CMakeCache

logger = logging.getLogger(__name__)


@pytest.fixture()
def gdb_process(dut: DeviceAdapter, gdb_script, gdb_timeout,
                gdb_target_remote) -> subprocess.CompletedProcess:
    build_dir = dut.device_config.build_dir
    cmake_cache = CMakeCache.from_file(os.path.join(build_dir, 'CMakeCache.txt'))
    gdb_exec = cmake_cache.get('CMAKE_GDB', None)
    assert gdb_exec, "CMAKE_GDB not found in CMakeCache"
    source_dir = cmake_cache.get('APPLICATION_SOURCE_DIR', None)
    assert source_dir
    build_image = cmake_cache.get('BYPRODUCT_KERNEL_ELF_NAME', None)
    assert build_image
    gdb_log_file = os.path.join(build_dir, 'gdb.log')
    cmd = [gdb_exec, '-batch',
           '-ex', 'set pagination off',
           '-ex', 'set trace-commands on',
           '-ex', f'set logging file {gdb_log_file}',
           '-ex', 'set logging enabled on',
           '-ex', f'target remote {gdb_target_remote}',
           '-x', f'{source_dir}/{gdb_script}', build_image]
    logger.info(f'Run GDB: {shlex.join(cmd)}')
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=gdb_timeout)
    logger.info(f'GDB ends rc={result.returncode}')
    logger.debug(f'GDB stdout:\n{result.stdout}')
    if result.stderr:
        logger.debug(f'GDB stderr:\n{result.stderr}')
    return result


@pytest.fixture(scope="module")
def expected_app():
    return [
        re.compile(r"main: starting GDB plugin test"),
        re.compile(r"worker: locking mutex"),
        re.compile(r"inspection_point: ready for GDB"),
    ]


@pytest.fixture(scope="module")
def expected_gdb():
    return [
        re.compile(r'Zephyr GDB plugin loaded'),
        re.compile(r'Address.*Name.*State'),
        re.compile(r'main'),
        re.compile(r'worker'),
        re.compile(r'k_mutex @ 0x[0-9a-fA-F]+'),
        re.compile(r'k_sem @ 0x[0-9a-fA-F]+'),
        re.compile(r'GDB_PLUGIN:PASSED'),
    ]


@pytest.fixture(scope="module")
def expected_gdb_detach():
    return [
        re.compile(r'Inferior.*will be killed'),
        re.compile(r'Inferior.*detached'),
    ]


def test_gdb_plugin(dut: DeviceAdapter, gdb_process, expected_app,
                    expected_gdb, expected_gdb_detach):
    """
    Test Zephyr GDB plugin RTOS-awareness commands.

    Connects GDB to QEMU gdbstub, loads the plugin, and verifies that
    thread listing, mutex inspection, and semaphore inspection produce
    expected output.
    """
    logger.debug(f"GDB output:\n{gdb_process.stdout}\n")
    assert gdb_process.returncode == 0, (
        f"GDB exited with rc={gdb_process.returncode}\n"
        f"stdout: {gdb_process.stdout}\n"
        f"stderr: {gdb_process.stderr}"
    )
    for ex_re in expected_gdb:
        assert ex_re.search(gdb_process.stdout, re.MULTILINE), (
            f"Expected pattern not found in GDB output: {ex_re.pattern}"
        )
    assert any(ex_re.search(gdb_process.stdout, re.MULTILINE)
               for ex_re in expected_gdb_detach), 'No expected GDB quit'
    app_output = '\n'.join(dut.readlines(print_output=False))
    logger.debug(f"App output:\n{app_output}\n")
    for ex_re in expected_app:
        assert ex_re.search(app_output, re.MULTILINE), (
            f"Expected pattern not found in app output: {ex_re.pattern}"
        )
#
