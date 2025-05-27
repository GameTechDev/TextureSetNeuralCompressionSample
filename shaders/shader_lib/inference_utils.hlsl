/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef INFERENCE_UTILS_HLSL
#define INFERENCE_UTILS_HLSL

// DXC Includes
#include "shader_lib/linalg.h"

// Resources
StructuredBuffer<float2> _UVOffsetBuffer: register(UV_OFFSET_BUFFER_BINDING);

#if defined(COOP_VECTOR_SUPPORTED)
ByteAddressBuffer _MLPWeight0Buffer: register(WEIGHT_0_BUFFER_BINDING);
ByteAddressBuffer _MLPBias0Buffer: register(WEIGHT_0_BIAS_BINDING);

ByteAddressBuffer _MLPWeight1Buffer: register(WEIGHT_1_BUFFER_BINDING);
ByteAddressBuffer _MLPBias1Buffer: register(WEIGHT_1_BIAS_BINDING);

ByteAddressBuffer _MLPWeight2Buffer: register(WEIGHT_2_BUFFER_BINDING);
ByteAddressBuffer _MLPBias2Buffer: register(WEIGHT_2_BIAS_BINDING);
#else
StructuredBuffer<float16_t> _MLPWeight0Buffer: register(WEIGHT_0_BUFFER_BINDING);
StructuredBuffer<float16_t> _MLPBias0Buffer: register(WEIGHT_0_BIAS_BINDING);

StructuredBuffer<float16_t> _MLPWeight1Buffer: register(WEIGHT_1_BUFFER_BINDING);
StructuredBuffer<float16_t> _MLPBias1Buffer: register(WEIGHT_1_BIAS_BINDING);

StructuredBuffer<float16_t> _MLPWeight2Buffer: register(WEIGHT_2_BUFFER_BINDING);
StructuredBuffer<float16_t> _MLPBias2Buffer: register(WEIGHT_2_BIAS_BINDING);
#endif

#if defined(LS_BC1_COMPRESSION)
	Texture2DArray<float4> _LS0Texture : register(LS0_BC1_BINDING);
	Texture2DArray<float4> _LS1Texture : register(LS1_BC1_BINDING);
	Texture2DArray<float4> _LS2Texture : register(LS2_BC1_BINDING);
	Texture2DArray<float4> _LS3Texture : register(LS3_BC1_BINDING);
	sampler bc1_linear_clamp_sampler: register(BC1_SAMPLER_BINDING);
#endif

#if defined(LS_BC1_COMPRESSION)
#if defined(COOP_VECTOR_SUPPORTED)
void sample_latent_space_bc1(out vector<float16_t, 16> coopVector, float2 uv, float2 uvDX, float2 uvDY, uint matID)
{
    float2 offsets = _UVOffsetBuffer[4 * matID];
    float3 ls0D = _LS0Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    coopVector[0] = float16_t(ls0D.x);
    coopVector[1] = float16_t(ls0D.y);
    coopVector[2] = float16_t(ls0D.z);

    offsets = _UVOffsetBuffer[4 * matID + 1];
    float3 ls1D = _LS1Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    coopVector[3] = float16_t(ls1D.x);
    coopVector[4] = float16_t(ls1D.y);
    coopVector[5] = float16_t(ls1D.z);

    offsets = _UVOffsetBuffer[4 * matID + 2];
    float3 ls2D = _LS2Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    coopVector[6] = float16_t(ls2D.x);
    coopVector[7] = float16_t(ls2D.y);
    coopVector[8] = float16_t(ls2D.z);

    offsets = _UVOffsetBuffer[4 * matID + 3];
    float3 ls3D = _LS3Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    coopVector[9] = float16_t(ls3D.x);
    coopVector[10] = float16_t(ls3D.y);
    coopVector[11] = float16_t(ls3D.z);
}
#endif

void sample_latent_space_bc1(out float16_t initialMemory[16], float2 uv, float2 uvDX, float2 uvDY, uint matID)
{
    float2 offsets = _UVOffsetBuffer[4 * matID];
    float3 ls0D = _LS0Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    initialMemory[0] = float16_t(ls0D.x);
    initialMemory[1] = float16_t(ls0D.y);
    initialMemory[2] = float16_t(ls0D.z);

    offsets = _UVOffsetBuffer[4 * matID + 1];
    float3 ls1D = _LS1Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    initialMemory[3] = float16_t(ls1D.x);
    initialMemory[4] = float16_t(ls1D.y);
    initialMemory[5] = float16_t(ls1D.z);

    offsets = _UVOffsetBuffer[4 * matID + 2];
    float3 ls2D = _LS2Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    initialMemory[6] = float16_t(ls2D.x);
    initialMemory[7] = float16_t(ls2D.y);
    initialMemory[8] = float16_t(ls2D.z);

    offsets = _UVOffsetBuffer[4 * matID + 3];
    float3 ls3D = _LS3Texture.SampleGrad(bc1_linear_clamp_sampler, float3(uv.xy + offsets, matID), uvDX, uvDY).xyz;
    initialMemory[9] = float16_t(ls3D.x);
    initialMemory[10] = float16_t(ls3D.y);
    initialMemory[11] = float16_t(ls3D.z);
}
#endif

#if defined(COOP_VECTOR_SUPPORTED)
void mlp_evaluation(inout vector<float16_t, MLP0_IN_DIM> inOutVec, uint matID)
{
    // First layer
    dx::linalg::MatrixRef<dx::linalg::DATA_TYPE_FLOAT16, MLP0_OUT_DIM, MLP0_IN_DIM, dx::linalg::MATRIX_LAYOUT_MUL_OPTIMAL> WeightMatrix0 = {_MLPWeight0Buffer, (MLP0_IN_DIM * MLP0_OUT_DIM) * matID * 2, 0};
    dx::linalg::VectorRef<dx::linalg::DATA_TYPE_FLOAT16> BiasVector0 = {_MLPBias0Buffer, MLP0_OUT_DIM * matID * 2};
    vector<float16_t, MLP0_OUT_DIM> tempVector = dx::linalg::MulAdd<float16_t>(WeightMatrix0, dx::linalg::MakeInterpretedVector<dx::linalg::DATA_TYPE_FLOAT16>(inOutVec), BiasVector0);
    
    // RELU
    tempVector = max(tempVector, 0);

    // Hidden layer
    dx::linalg::MatrixRef<dx::linalg::DATA_TYPE_FLOAT16, MLP1_OUT_DIM, MLP0_OUT_DIM, dx::linalg::MATRIX_LAYOUT_MUL_OPTIMAL> WeightMatrix1 = {_MLPWeight1Buffer, (MLP0_OUT_DIM * MLP1_OUT_DIM) * matID * 2, 0};
    dx::linalg::VectorRef<dx::linalg::DATA_TYPE_FLOAT16> BiasVector1 = {_MLPBias1Buffer, MLP1_OUT_DIM * matID * 2};
    tempVector = dx::linalg::MulAdd<float16_t>(WeightMatrix1, dx::linalg::MakeInterpretedVector<dx::linalg::DATA_TYPE_FLOAT16>(tempVector), BiasVector1);
    
    // RELU
    tempVector = max(tempVector, 0);

    // Third layer
    dx::linalg::MatrixRef<dx::linalg::DATA_TYPE_FLOAT16, MLP2_OUT_DIM, MLP1_OUT_DIM, dx::linalg::MATRIX_LAYOUT_MUL_OPTIMAL> WeightMatrix2 = {_MLPWeight2Buffer, (MLP1_OUT_DIM * MLP2_OUT_DIM) * matID * 2, 0};
    dx::linalg::VectorRef<dx::linalg::DATA_TYPE_FLOAT16> BiasVector2 = {_MLPBias2Buffer, MLP2_OUT_DIM * matID * 2};
    inOutVec = dx::linalg::MulAdd<float16_t>(WeightMatrix2, dx::linalg::MakeInterpretedVector<dx::linalg::DATA_TYPE_FLOAT16>(tempVector), BiasVector2);
}
#endif

#if !defined(COOP_VECTOR_SUPPORTED)
void mlp_evaluation(inout float16_t initialMemory[16], uint matID)
{
    // Do the mat mul
    float16_t pongMemoryA[MLP0_OUT_DIM];
    [unroll] for (uint32_t x = 0; x < MLP0_OUT_DIM; ++x)
    {
        float16_t acc = float16_t(0.0);
        [unroll] for (uint32_t l = 0; l < MLP0_IN_DIM; ++l)
            acc = fma(initialMemory[l], _MLPWeight0Buffer[MLP0_OUT_DIM * l + x + (MLP0_IN_DIM * MLP0_OUT_DIM) * matID], acc);

        // Add the bias
        acc += _MLPBias0Buffer[x + MLP0_OUT_DIM * matID];
        pongMemoryA[x] = max(acc, float16_t(0.0));
    }

    // Do the mat mul
    float16_t pongMemoryB[MLP1_OUT_DIM];
    [unroll]  for (uint32_t x = 0; x < MLP1_OUT_DIM; ++x)
    {
        float16_t acc = float16_t(0.0);
        [unroll] for (uint32_t l = 0; l < MLP0_OUT_DIM; ++l)
            acc = fma(pongMemoryA[l], _MLPWeight1Buffer[MLP1_OUT_DIM * l + x + (MLP0_OUT_DIM * MLP1_OUT_DIM) * matID], acc);

        // Add the bias
        acc += _MLPBias1Buffer[x + MLP1_OUT_DIM * matID];
        pongMemoryB[x] = max(acc, float16_t(0.0));
    }

    [unroll] for (uint32_t x = 0; x < MLP2_OUT_DIM; ++x)
    {
        // Do the mat mul
        float16_t acc = float16_t(0.0);
        [unroll] for (uint32_t l = 0; l < MLP1_OUT_DIM; ++l)
            acc = fma(pongMemoryB[l], _MLPWeight2Buffer[MLP2_OUT_DIM * l + x + (MLP1_OUT_DIM * MLP2_OUT_DIM) * matID], acc);

        // Add the bias
        acc += _MLPBias2Buffer[x + MLP2_OUT_DIM * matID];
        initialMemory[x] = acc;
    }
}
#endif

#endif // INFERENCE_UTILS_HLSL