/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "math/types.h"

struct GlobalCB
{
    // View projection matrix
    float4x4 _ViewProjectionMatrix;

    // Inverse view projection matrix
    float4x4 _InvViewProjectionMatrix;

    // Camera position in double as we are operating on planeraty coordinates
    float3 _CameraPosition;
    float _EnablePP;

    // Screen size
    uint2 _ScreenSize;
    // Texture size
    uint2 _TextureSize;

    // Tile size
    uint2 _TileSize;
    // Interpolation factor for skinning
    float _AnimationFactor;
    // Number of vertices of our mesh
    uint32_t _MeshNumVerts;

    // Filtering
    float _EnableFiltering;
    float2 _PaddingGB0;
    uint32_t _FrameIndex;

    // Sun direction
    float3 _SunDirection;
    uint32_t _MLPCount;

    // Debug channels
    uint32_t _ChannelSet;
    float2 _NumTextureLOD;
    float _AnimationTime;
};