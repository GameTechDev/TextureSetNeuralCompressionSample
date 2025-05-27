/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "tools/directory_utilities.h"
#include "tools/security.h"

// System includes
#include <fstream>

void list_files_by_extension(const char* directoryPath, const char* extension, std::vector<std::string>& fileNameArray)
{
    // Loop through the files in the model library
    DIR* d;
    struct dirent* dir;
    d = opendir(directoryPath);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            std::string fileName = dir->d_name;
            if (fileName.find(extension) != std::string::npos)
                fileNameArray.push_back(fileName);
        }
        closedir(d);
    }
}

void load_file_to_array(const char* filePath, std::vector<char>& outData)
{
    // Read the file to a buffer
    std::ifstream nnFile(filePath, std::ios::binary | std::ios::ate);
    assert_msg(nnFile.is_open(), "Failed to open pixel network\n");
    std::streamsize nnSize = nnFile.tellg();
    nnFile.seekg(0, std::ios::beg);
    outData.resize(nnSize);
    assert(nnFile.read(outData.data(), nnSize));
}