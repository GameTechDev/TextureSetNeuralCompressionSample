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

# Point to the local includes
set(D3D12_INCLUDE_DIRS "${BACASABLE_SDK_INCLUDES}")

# List of required libraries
set(D3D12_LIBRARIES d3d12.lib dxgi.lib d3dcompiler.lib)

# Handle the QUIETLY and REQUIRED arguments and set D3D12_FOUND to TRUE
include(FindPackageHandleStandardArgs)

# if all listed variables are TRUE
find_package_handle_standard_args(D3D12 DEFAULT_MSG D3D12_INCLUDE_DIRS D3D12_LIBRARIES)

# Hide from UI
mark_as_advanced(D3D12_INCLUDE_DIRS D3D12_LIBRARIES)