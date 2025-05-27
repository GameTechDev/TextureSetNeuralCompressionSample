/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef MATERIAL_HLSL
#define MATERIAL_HLSL

struct SurfaceData
{
    float3 baseColor;
    float3 normalTS;
    float ambientOcclusion;
    float perceptualRoughness;
    float metalness;
    float thickness;
    float2 mask;
};

struct GeometryData
{
    float3 geoNormalWS;
    float3 tangentWS;
    float3 viewWS;
};

#endif // MATERIAL_HLSL