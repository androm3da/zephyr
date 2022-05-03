#!/bin/bash

#tools=/pkg/qct/software/llvm/build_tools/clang+llvm-13.0.0-cross-hexagon-unknown-linux-musl/
hexagon_tools=/pkg/qct/software/hexagon/releases/tools/8.6.02/Tools/
#hexagon_tools=/pkg/qct/software/hexagon/releases/tools/8.5.11/Tools/
#tools=/prj/qct/llvm/release/internal/HEXAGON/branch-8.6lnx/latest/Tools/
#export PATH=/pkg/qct/software/llvm/build_tools/clang+llvm-13.0.0-cross-x86_64-linux-gnu-ubuntu-16.04/bin:${PATH}
#export PATH=/pkg/qct/software/llvm/build_tools/clang+llvm-13.0.0-cross-hexagon-unknown-linux-musl/x86_64-linux-gnu/bin:${PATH}
export PATH=${hexagon_tools}/bin:${PATH}

export ZEPHYR_BASE=${PWD}
#   --trace --trace-expand \

/pkg/qct/software/cmake/3.20.1/bin/cmake -GNinja \
    -B${PWD}/build \
    -DBOARD=qemu_hexagon_virt \
    -DLLVM_TOOLCHAIN_PATH=${hexagon_tools} \
    -C./cmake/toolchain/llvm/hexagon-downstream.cmake \
    ${PWD}/samples/hello_world
ninja -v -C build
