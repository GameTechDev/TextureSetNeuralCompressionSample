/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef TRANSFORM_HLSL
#define TRANSFORM_HLSL

// NDC to clip Space
float4 evaluate_clip_space_position(float2 positionNDC, float depthValue)
{
    return float4(positionNDC * 2.0 - 1.0, depthValue, 1.0);
}

float3 evaluate_world_space_position(float2 positionNDC, float depthValue, float4x4 invViewProjMatrix)
{
    float4 positionCS  = evaluate_clip_space_position(positionNDC, depthValue);
    float4 hpositionWS = mul(invViewProjMatrix, positionCS);
    return hpositionWS.xyz / hpositionWS.w;
}

float4 evaluate_homogenous_position(float3 positionRWS, float4x4 viewProjMatrix)
{
    return mul(viewProjMatrix, float4(positionRWS, 1.0));
}

float2 normalized_coordinates_to_longlat(float3 positionNPS)
{
    float lat = asin(positionNPS.y) / PI + 0.5;
    float lon = atan2(-positionNPS.x,  positionNPS.z) / (2.0 * PI) + 0.5;
    return float2(lon, lat);
}

float2 evaluate_ndc_coordinates(float2 pixelCoords, float2 screenSize)
{
    float2 ndc = (pixelCoords / float2(_ScreenSize.xy)) * 2.0 - 1.0;
    ndc.y = - ndc.y;
    return ndc;
}

float3x3 get_local_frame(float3 localZ)
{
    float x  = localZ.x;
    float y  = localZ.y;
    float z  = localZ.z;
    float sz = sign(z);
    float a  = 1 / (sz + z);
    float ya = y * a;
    float b  = x * ya;
    float c  = x * sz;

    float3 localX = float3(c * x * a - 1, sz * b, c);
    float3 localY = float3(b, y * ya - sz, y);

    // Note: due to the quaternion formulation, the generated frame is rotated by 180 degrees,
    // s.t. if localZ = {0, 0, 1}, then localX = {-1, 0, 0} and localY = {0, -1, 0}.
    return float3x3(localX, localY, localZ);
}

float3x3 get_local_frame(float3 localZ, float3 localX)
{
    float3 localY = cross(localZ, localX);
    return float3x3(localX, localY, localZ);
}

float3 combine_normals(float3 normalWS, float3 tangentWS, float3 normalTS)
{
	normalTS = normalize(normalTS * 2.0 - 1.0);
	float3x3 TBN = get_local_frame(normalWS, tangentWS);
    return normalize(mul(normalTS, TBN));
}

#endif // TRANSFORM_HLSL