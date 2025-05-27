/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef COLOR_HLSL
#define COLOR_HLSL

float3 srgb_to_linear(float3 c)
{
    float3 linearRGBLo = c / 12.92;
    float3 linearRGBHi = positive_pow((c + (0.055)) / (1.055), float3(2.4, 2.4, 2.4));
    float3 linearRGB = float3((c.x <= 0.04045) ? linearRGBLo.x : linearRGBHi.x, (c.y <= 0.04045) ? linearRGBLo.y : linearRGBHi.y, (c.z <= 0.04045) ? linearRGBLo.z : linearRGBHi.z);
    return linearRGB;
}

float3 linear_to_srgb(float3 c)
{
    float3 sRGBLo = c * 12.92;
    float3 sRGBHi = (positive_pow(c, float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * (1.055)) - (0.055);
    float3 sRGB = float3((c.x <= 0.0031308) ? sRGBLo.x : sRGBHi.x, (c.y <= 0.0031308) ? sRGBLo.y : sRGBHi.y, (c.z <= 0.0031308) ? sRGBLo.z : sRGBHi.z);
    return sRGB;
}

float3 chroma_color(float t)
{
    if (t < 0.33)
        return float3(0.1, 0.3, 1.0);
    if (t > 0.666)
        return float3(1.0, 0.3, 1.0);
    return float3(0.0, 0.0, 1.0);
}

#endif //COLOR_HLSL