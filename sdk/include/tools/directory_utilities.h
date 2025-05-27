/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "tools/dirent.h"

// System includes
#include <vector>
#include <string>

void list_files_by_extension(const char* directoryPath, const char* extension, std::vector<std::string>& fileNameArray);

// Load a file to a char array
void load_file_to_array(const char* filePath, std::vector<char>& outData);