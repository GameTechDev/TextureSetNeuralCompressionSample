#
# Copyright(c) 2025 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Minimal cmake version
cmake_minimum_required(VERSION 3.5)

# This module is shared; use include blocker.
if( _PLATFORMS_ )
	return()
endif()

# Mark it as processed
set(_PLATFORMS_ 1)

# Detect target platform
if(NOT WIN32)
    message(FATAL_ERROR "This project must be built on Windows.")
endif()

set(PLATFORM_WINDOWS 1)
set(PLATFORM_NAME "windows")
add_definitions(-DWINDOWSPC)

message(STATUS "Detected platform: ${PLATFORM_NAME}")

# Set the target architecture
set(PLATFORM_64BIT 1)

# Configure CMake global variables
set(CMAKE_INSTALL_MESSAGE LAZY)

# Set the output folders based on the identifier
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/bin)
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)

# Find D3D12 and enable it if possible
FIND_PACKAGE(D3D12)
add_definitions(-DD3D12_SUPPORTED)
add_definitions(-DD3D12_EXPERIMENTAL_COOP_VECTOR)
add_definitions(-DD3D12_EXPERIMENTAL_SHADER_MODEL)
if (NOT DEFINED DDX12_SDK_VERSION)
	add_definitions(-DDX12_SDK_VERSION=717)
endif()