/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef RANDOM_HLSL
#define RANDOM_HLSL

// See: http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
uint Hash(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27d4eb2du;
    seed = seed ^ (seed >> 15u);
    return seed;
}

uint BitReverse(uint x)
{
    x = ((x & 0x55555555u) <<  1u) | ((x & 0xAAAAAAAAu) >>  1u);
    x = ((x & 0x33333333u) <<  2u) | ((x & 0xCCCCCCCCu) >>  2u);
    x = ((x & 0x0F0F0F0Fu) <<  4u) | ((x & 0xF0F0F0F0u) >>  4u);
    x = ((x & 0x00FF00FFu) <<  8u) | ((x & 0xFF00FF00u) >>  8u);
    x = ((x & 0x0000FFFFu) << 16u) | ((x & 0xFFFF0000u) >> 16u);

    return x;
}

uint Dilate(uint x) {
    x = (x | (x << 8)) & 0x00FF00FFu;
    x = (x | (x << 4)) & 0x0F0F0F0Fu;
    x = (x | (x << 2)) & 0x33333333u;
    x = (x | (x << 1)) & 0x55555555u;

    return x;
}

uint BayerMatrixCoefficient(uint i, uint j, uint matrixSize)
{
    uint x = i ^ j;
    uint y = j;
    uint z = Dilate(x) | (Dilate(y) << 1);
    uint b = BitReverse(z) >> (32 - (matrixSize << 1));

    return b;
}

uint IRng(uint rngState)
{
    // LCG values from Numerical Recipes
    return 1664525u * rngState + 1013904223u;
}

// Random float in range [0, 1)
// Implementation adapted from ispc
float URng(inout uint seed)
{
    uint tmp = IRng(seed);
    uint irn = tmp & ((1u << 23) - 1u);

    seed = tmp;

    return asfloat(0x3F800000u | irn) - 1.0f;
}

uint32_t pixel_seed(uint2 coordinates, float2 screenSize, uint32_t frameIndex)
{
    return Hash(BayerMatrixCoefficient(coordinates.x & 255, coordinates.y & 255, 16) + frameIndex);
}

#endif // RANDOM_HLSL