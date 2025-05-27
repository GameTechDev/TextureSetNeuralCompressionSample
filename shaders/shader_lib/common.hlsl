/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef COMMON_HLSL
#define COMMON_HLSL

// Constants
#define HALF_MAX        65504.0 // (2 - 2^-10) * 2^15
#define HALF_MAX_MINUS1 65472.0 // (2 - 2^-9) * 2^15
#define EPSILON         1.0e-4
#define PI              3.14159265359
#define TWO_PI          6.28318530718
#define FOUR_PI         12.56637061436
#define INV_PI          0.31830988618
#define INV_TWO_PI      0.15915494309
#define INV_FOUR_PI     0.07957747155
#define HALF_PI         1.57079632679
#define INV_HALF_PI     0.636619772367
#define FLT_EPSILON     1.192092896e-07 // Smallest positive number, such that 1.0 + FLT_EPSILON != 1.0
#define FLT_MIN         1.175494351e-38 // Minimum representable positive floating-point number
#define FLT_MAX         3.402823466e+38 // Maximum representable floating-point number

// Sizes and macros
#define WORK_GROUP_SIZE 32
#define INDIRECT_LIGHTING_MULTIPLIER 0.4
#define FIXED_EXPOSURE 5.0
#define FINAL_GAMMA 1.8

// Material data
#define AO_OFFSET 0
#define DIFFUSE_OFFSET 1
#define DISPLACEMENT_OFFSET 4
#define MASK_OFFSET 5
#define METALNESS_OFFSET 7
#define NORMAL_OFFSET 8
#define ROUGHNESS_OFFSET 11
#define THICKNESS_OFFSET 12
#define SSS_COLOR float3(0.6, 0.4, 0.21)

uint packHalf2x16(float2 v)
{
    return f32tof16(v.x) | (f32tof16(v.y) << 16);
}

float2 unpackHalf2x16(uint v)
{
    return float2(f16tof32(v & 0xffff), f16tof32((v >> 16)& 0xffff));
}

float Sq(float v)
{
    return v*v;
}

float16_t fma(float16_t a, float16_t b, float16_t c)
{
    return a * b + c;
}

float positive_pow(float base, float power)
{
    return pow(max(abs(base), float(FLT_EPSILON)), power);
}

float3 positive_pow(float3 base, float3 power)
{
    return pow(max(abs(base), FLT_EPSILON), power);
}

float checker_board(float2 UV)
{
    float2 distance3 = 4.0 * abs(frac(UV + 0.25) - 0.5) - 1.0;
    float4 derivatives = float4(ddx(UV), ddy(UV));
    float2 duv_length = sqrt(float2(dot(derivatives.xz, derivatives.xz), dot(derivatives.yw, derivatives.yw)));
    float2 scale = 0.35 / duv_length.xy;
    float freqLimiter = sqrt(clamp(1.1f - max(duv_length.x, duv_length.y), 0.0, 1.0));
    float2 vector_alpha = clamp(distance3 * scale.xy, -1.0, 1.0);
    return saturate(0.5f + 0.5f * vector_alpha.x * vector_alpha.y * freqLimiter);
}

float4 vertex_id_to_cs_pos(uint vertexID)
{
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    return float4(uv * 2.0 - 1.0, 0.0, 1.0);
}

uint2 pixel_coords_from_work_group_id(uint32_t workGroupID, uint2 groupThreadID, uint2 tileSize)
{
    // Get it as coordinates
    uint wgX = workGroupID % tileSize.x;
    uint wgY = workGroupID / tileSize.x;

    // Compute the pixel coords
    return uint2(wgX * 8 + groupThreadID.x, wgY * 4 + groupThreadID.y);
}

#endif // COMMON_HLSL