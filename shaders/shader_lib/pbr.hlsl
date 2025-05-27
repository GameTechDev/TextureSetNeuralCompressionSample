/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef PBR_HLSL
#define PBR_HLSL

// Ref: Steve McAuley - Energy-Conserving Wrapped Diffuse
float compute_wrapped_diffuse_lighting(float NdotL, float w)
{
    return clamp((NdotL + w) / ((1.0 + w) * (1.0 + w)), 0.0, 1.0);
}

float compute_f90(float3 F0)
{
    float dotF = dot(F0, float3(0.333, 0.333, 0.333));
    return clamp(50.0 * dotF, 0.0, 1.0);
}

float f_schlick(float f0, float f90, float u)
{
    float x = 1.0 - u;
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return (f90 - f0) * x5 + f0;
}

float f_schlick(float f0, float u)
{
    return f_schlick(f0, 1.0, u);
}

float3 f_schlick(float3 f0, float f90, float u)
{
    float x = 1.0 - u;
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return f0 * (1.0 - x5) + (f90 * x5);
}

float3 f_schlick(float3 f0, float u)
{
    return f_schlick(f0, 1.0, u);
}

float perceptual_roughness_to_roughness(float perceptualRoughness)
{
    return perceptualRoughness * perceptualRoughness;
}

float roughness_to_perceptual_roughness(float roughness)
{
    return sqrt(roughness);
}

float perceptual_roughness_to_mipmap_level(float perceptualRoughness, float maxMipLevel)
{
    perceptualRoughness = perceptualRoughness * (1.7 - 0.7 * perceptualRoughness);
    return clamp(perceptualRoughness * maxMipLevel, 0.0, 6.0);
}

float3 compute_diffuse_color(float3 baseColor, float metallic)
{
    return baseColor * (1.0 - metallic);
}

float3 compute_fresnel0(float3 baseColor, float metallic, float dielectricF0)
{
    return lerp(dielectricF0.xxx, baseColor, metallic);
}

#endif // PBR_HLSL