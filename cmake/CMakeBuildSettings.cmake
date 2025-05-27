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

macro(define_plaform_settings)
	add_compile_options(/Zi)
	add_compile_options($<$<CONFIG:DEBUG>:/Od> $<$<NOT:$<CONFIG:DEBUG>>:/Ox>)
	add_compile_options(/Ob2)
	add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/Oi>)
	add_compile_options(/Ot)
	add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/GT>)
	add_compile_options(/GF)

	if( PLATFORM_WINDOWS AND RUNTIME_TYPE STREQUAL "mt")
		add_compile_options($<$<CONFIG:DEBUG>:/MTd> $<$<NOT:$<CONFIG:DEBUG>>:/MT>)
	elseif( PLATFORM_WINDOWS AND RUNTIME_TYPE STREQUAL "md")
		add_compile_options($<$<CONFIG:DEBUG>:/MDd> $<$<NOT:$<CONFIG:DEBUG>>:/MD>)
	endif()

	add_compile_options(/Gy)
	add_compile_options(/fp:fast)
	replace_compile_flags("/GR" "/GR-")

	add_compile_options(/W4)
	add_compile_options(/WX)

	add_exe_linker_flags(/DEBUG)
	add_exe_linker_flags(/MAP)
	replace_linker_flags("/INCREMENTAL" "/INCREMENTAL:NO" debug)
	add_compile_options(/MP)
	add_compile_options(-D_HAS_EXCEPTIONS=0)
	replace_linker_flags("/debug" "/DEBUG" debug)
	replace_linker_flags("/machine:x64" "/MACHINE:X64")
	add_compile_options(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
	add_compile_options(-DSECURITY_WIN32)
	
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
endmacro()
