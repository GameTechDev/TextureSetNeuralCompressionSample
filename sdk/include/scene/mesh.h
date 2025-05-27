/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Project includes
#include "graphics/types.h"

// System includes
#include <vector>

struct VertexBuffer
{
    std::vector<VertexData> data;
};

struct MeshAnimation
{
    std::vector<uint3> indexBuffer;
    std::vector<VertexBuffer> vertexBufferArray;
};

namespace mesh
{
    // Import a packed mesh animation from disk
    void import_mesh_animation(const char* path, MeshAnimation& meshAnimation);

    // Export a packed mesh animation to disk
    void export_mesh_animation(const MeshAnimation& meshAnimation, const char* path);
}