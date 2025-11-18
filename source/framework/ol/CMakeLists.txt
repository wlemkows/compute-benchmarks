#
# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

# OL (Offload API) Framework Configuration
#
# This module requires the LLVM Offload Runtime library and headers.
# Both OFFLOAD_INSTALL_DIR and OFFLOAD_INCLUDE_DIR must be set.
#
# OFFLOAD_INSTALL_DIR:
#   Path to the directory containing libLLVMOffload library.
#   Example: /llvm-project/build/lib
#
# OFFLOAD_INCLUDE_DIR:
#   Path to the directory containing OffloadAPI.h header.
#   Example: /llvm-project/build/runtimes/runtimes-bins/offload/liboffload/API
#
# Usage Example:
#
#   cmake -DBUILD_OL=ON \
#         -DOFFLOAD_INSTALL_DIR=/path/to/llvm-project/build/lib \
#         -DOFFLOAD_INCLUDE_DIR=/path/to/llvm-project/build/runtimes/runtimes-bins/offload/liboffload/API \
#         ..
#

if (NOT BUILD_OL)
    return()
endif()

# User must specify offload library directory
if(NOT DEFINED OFFLOAD_INSTALL_DIR)
    set(OFFLOAD_INSTALL_DIR "" CACHE PATH "Path to libLLVMOffload library directory (e.g., llvm-project/build/lib)")
endif()

# User must specify offload include directory
if(NOT DEFINED OFFLOAD_INCLUDE_DIR)
    set(OFFLOAD_INCLUDE_DIR "" CACHE PATH "Path to OffloadAPI.h directory (e.g., llvm-project/build/runtimes/runtimes-bins/offload/liboffload/API)")
endif()

# Check if both paths are set
if(OFFLOAD_INSTALL_DIR STREQUAL "" OR OFFLOAD_INCLUDE_DIR STREQUAL "")
    message(FATAL_ERROR
        "Offload paths are not set. Both variables are required:\n"
        "  - OFFLOAD_INSTALL_DIR (path to libLLVMOffload library directory)\n"
        "  - OFFLOAD_INCLUDE_DIR (path to OffloadAPI.h directory)\n\n"
        "Example:\n"
        "  cmake -DBUILD_OL=ON \\\n"
        "        -DOFFLOAD_INSTALL_DIR=/path/to/llvm-project/build/lib \\\n"
        "        -DOFFLOAD_INCLUDE_DIR=/path/to/llvm-project/build/runtimes/runtimes-bins/offload/liboffload/API \\\n"
        "        ..")
endif()

# Find the library in the install directory
find_library(OFFLOAD_LIBRARY 
    NAMES LLVMOffload
    PATHS ${OFFLOAD_INSTALL_DIR}
    NO_DEFAULT_PATH
)

if(OFFLOAD_LIBRARY AND EXISTS ${OFFLOAD_INCLUDE_DIR}/OffloadAPI.h)
    message(STATUS "libLLVMOffload found at:    ${OFFLOAD_LIBRARY}")
    message(STATUS "OffloadAPI.h found at:      ${OFFLOAD_INCLUDE_DIR}/OffloadAPI.h")
    
    # Create imported targets
    add_library(libLLVMOffload::offload UNKNOWN IMPORTED)
    set_target_properties(libLLVMOffload::offload PROPERTIES
        IMPORTED_LOCATION "${OFFLOAD_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OFFLOAD_INCLUDE_DIR}"
    )
    set(libLLVMOffload_FOUND TRUE)
else()
    message(STATUS "libLLVMOffload NOT found at: ${OFFLOAD_INSTALL_DIR}")
    message(STATUS "OffloadAPI.h search path:    ${OFFLOAD_INCLUDE_DIR}/OffloadAPI.h")
    set(libLLVMOffload_FOUND FALSE)
endif()

if(NOT libLLVMOffload_FOUND)
    # TODO: handle liboffload fetch content and build
    message(FATAL_ERROR
        "libLLVMOffload not found, please setup manually and ensure:\n"
        "  1. LLVM is built with Offload Runtime enabled\n"
        "  2. Paths are correct:\n"
        "     - Library directory:  ${OFFLOAD_INSTALL_DIR}\n"
        "     - Include directory:  ${OFFLOAD_INCLUDE_DIR}\n\n")
endif()

# Define target
set(API_NAME ol)

set(TARGET_NAME compute_benchmarks_framework_${API_NAME})
add_library(${TARGET_NAME} STATIC error.h ol.h ol.cpp)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OFFLOAD_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} PUBLIC compute_benchmarks_framework libLLVMOffload::offload)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER framework)
setup_vs_folders(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
setup_warning_options(${TARGET_NAME})
setup_output_directory(${TARGET_NAME})
if (MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Add this API to global array
set_property(GLOBAL APPEND PROPERTY APIS ${API_NAME})
