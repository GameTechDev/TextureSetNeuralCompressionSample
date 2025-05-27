/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// SDK incldues
#include "graphics/descriptors.h"

struct BinaryTexture
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipCount;
    TextureFormat format;
    TextureType type;
    std::vector<uint8_t> data;
};

// Our packed BC1 and BC6 formats
GraphicsBuffer load_bc1_to_graphics_buffer(GraphicsDevice device, const char* texturePath, uint3& dimensions, float2& uvOffset);
GraphicsBuffer load_bc6_to_graphics_buffer(GraphicsDevice device, const char* texturePath, uint32_t& width, uint32_t& height, uint32_t& mipCount);

namespace binary_texture
{
    void import_binary_texture(const char* path, BinaryTexture& bt);
    void export_binary_texture(const BinaryTexture& bt, const char* path);
}
