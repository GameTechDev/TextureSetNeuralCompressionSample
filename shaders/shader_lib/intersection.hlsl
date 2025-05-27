/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef INTERSECTION_HLSL
#define INTERSECTION_HLSL

// Intersection function that only returns valid intersection for the front face
float intersect_plane(float3 rayOrigin, float3 rayDir, float3 planeNormal)
{
    float denom = dot(rayDir, planeNormal);
    if (denom > 1e-6) 
        return -1.0f;
    return dot(-rayOrigin, planeNormal) / denom;
}
#endif // INTERSECTION_HLSL