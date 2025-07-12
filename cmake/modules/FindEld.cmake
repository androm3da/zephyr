# SPDX-License-Identifier: Apache-2.0
#
# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

# FindEld module for locating ELD linker.
#
# The module defines the following variables:
#
# 'ELD_LINKER'
# Path to ELD linker
# Set to 'ELD_LINKER-NOTFOUND' if eld was not found.
#
# 'Eld_FOUND', 'ELD_FOUND'
# True if ELD was found.
#
# 'ELD_VERSION_STRING'
# The version of ELD.

include(FindPackageHandleStandardArgs)

# See if the compiler has a preferred linker
execute_process(COMMAND ${CMAKE_C_COMPILER} --print-prog-name=ld.eld
                OUTPUT_VARIABLE ELD_LINKER
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT EXISTS "${ELD_LINKER}")
  # Need to clear it or else find_program() won't replace the value.
  set(ELD_LINKER)

  if(DEFINED TOOLCHAIN_HOME)
    # Search for linker under TOOLCHAIN_HOME if it is defined
    # to limit which linker to use, or else we would be using
    # host tools.
    set(ELD_SEARCH_PATH PATHS ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  endif()

  find_program(ELD_LINKER ld.eld ${ELD_SEARCH_PATH})
endif()

if(ELD_LINKER)
  # Parse the 'eld --version' output to find the installed version.
  execute_process(
    COMMAND
    ${ELD_LINKER} --version
    OUTPUT_VARIABLE eld_version_output
    ERROR_VARIABLE  eld_error_output
    RESULT_VARIABLE eld_status
    )

  set(ELD_VERSION_STRING)
  if(${eld_status} EQUAL 0)
    # Extract ELD version - if available
    if(eld_version_output)
      string(REGEX MATCH "ELD ([0-9]+[.][0-9]+[.]?[0-9]*).*" out_var "${eld_version_output}")
      if(CMAKE_MATCH_1)
        set(ELD_VERSION_STRING ${CMAKE_MATCH_1})
      else()
        # Fallback version if no version string found
        set(ELD_VERSION_STRING "1.0.0")
      endif()
    else()
      set(ELD_VERSION_STRING "1.0.0")
    endif()
  endif()
endif()

find_package_handle_standard_args(Eld
                                  REQUIRED_VARS ELD_LINKER
                                  VERSION_VAR ELD_VERSION_STRING
)