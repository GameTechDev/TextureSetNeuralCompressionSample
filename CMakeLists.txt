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

# Project name
project(tsnc_coop)

# 3rd dependencies
set(PROJECT_3RD_INCLUDES ${PROJECT_SOURCE_DIR}/3rd/include)
set(PROJECT_3RD_LIBRARY ${PROJECT_SOURCE_DIR}/3rd/lib)
set(PROJECT_3RD_BINARY ${PROJECT_SOURCE_DIR}/3rd/bin)

# Declare the global variables for the cmake project
set(SDK_ROOT ${PROJECT_SOURCE_DIR}/sdk)
set(SDK_INCLUDE ${SDK_ROOT}/include)

# Set the cmake path variable
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# This flag must be activated in order to handle properly folder flags
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define the cmake macros
include(CMakePlatforms)
include(CMakeMacros)
include(CMakeBuildSettings)

# Define the build options
define_plaform_settings()

# Print the platform's name
message(STATUS "The build identifier is: ${BACASABLE_PLATFORM_NAME}")

# Create the list of allowed files to be included
set(bacasable_source_extensions)
list(APPEND bacasable_source_extensions ".h" ".cpp" ".inl" ".txt")

# Generate the gpu_mesh SDK
add_subdirectory(${SDK_ROOT}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/project)
