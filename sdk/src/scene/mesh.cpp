/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "scene/mesh.h"
#include "tools/stream.h"

namespace mesh
{
    void import_mesh_animation(const char* path, MeshAnimation& meshAnimation)
    {
        // Vector that will hold our packed meshAnimation 
        std::vector<char> binaryFile;

        // Read from disk
        FILE* pFile;
        pFile = fopen(path, "rb");
        fseek(pFile, 0L, SEEK_END);
        size_t fileSize = _ftelli64(pFile);
        binaryFile.resize(fileSize);
        _fseeki64(pFile, 0L, SEEK_SET);
        rewind(pFile);
        fread(binaryFile.data(), sizeof(char), fileSize, pFile);
        fclose(pFile);

        // Pack the structure in a buffer
        const char* binaryPtr = binaryFile.data();

        // Read the index buffers
        unpack_vector_bytes(binaryPtr, meshAnimation.indexBuffer);

        // Read the number of frames
        uint32_t numFrames;
        unpack_bytes(binaryPtr, numFrames);

        // Read the vertex buffers
        meshAnimation.vertexBufferArray.resize(numFrames);
        for (uint32_t idx = 0; idx < numFrames; ++idx)
        {
            unpack_vector_bytes(binaryPtr, meshAnimation.vertexBufferArray[idx].data);
        }
    }

    void export_mesh_animation(const MeshAnimation& meshAnimation, const char* path)
    {
        // Vector that will hold our packed mesh 
        std::vector<char> binaryFile;

        // Write the index buffer
        pack_vector_bytes(binaryFile, meshAnimation.indexBuffer);

        // Write the number of frames
        uint32_t numFrames = (uint32_t)meshAnimation.vertexBufferArray.size();
        pack_bytes(binaryFile, numFrames);

        // Write the vertex buffers
        for (uint32_t idx = 0; idx < numFrames; ++idx)
            pack_vector_bytes(binaryFile, meshAnimation.vertexBufferArray[idx].data);

        // Write to disk
        FILE* pFile;
        pFile = fopen(path, "wb");
        fwrite(binaryFile.data(), sizeof(char), binaryFile.size(), pFile);
        fclose(pFile);
    }
}