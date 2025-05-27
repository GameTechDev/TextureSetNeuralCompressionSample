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

# Exe declaration
bacasable_exe(dino_danger "projects" "dino_danger.cpp" "${SDK_INCLUDE}")

# Libraries
target_link_libraries(dino_danger "sdk" "${D3D12_LIBRARIES}")
target_link_libraries(dino_danger "${PROJECT_3RD_LIBRARY}/dxcompiler.lib")
target_link_libraries(dino_danger "${PROJECT_3RD_LIBRARY}/dxil.lib")

# DLLS
copy_next_to_binary(dino_danger "${PROJECT_3RD_BINARY}/dxcompiler.dll")
copy_next_to_binary(dino_danger "${PROJECT_3RD_BINARY}/dxil.dll")
copy_dir_next_to_binary(dino_danger "${PROJECT_SOURCE_DIR}/3rd/bin/D3D12" "D3D12")

# Parameters
set_target_properties(dino_danger PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--data-dir ${PROJECT_SOURCE_DIR}")