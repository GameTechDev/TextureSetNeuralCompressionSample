/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef MESH_UTILITIES_HLSL
#define MESH_UTILITIES_HLSL

// SPIRV-Compliant data
struct VertexData
{
    float4 data0;
    float4 data1;
    float4 data2;
};

#if defined(VERTEX_DATA_BUFFER_BINDING)
StructuredBuffer<VertexData> _VertexBuffer: register(VERTEX_DATA_BUFFER_BINDING);
#endif

#if defined(INDEX_BUFFER_BINDING)
StructuredBuffer<uint> _IndexBuffer: register(INDEX_BUFFER_BINDING);
#endif

float3 position(in VertexData vData)
{
    return float3(vData.data0.x, vData.data0.y, vData.data0.z);
}

float3 normal(in VertexData vData)
{
    return float3(vData.data0.w, vData.data1.x, vData.data1.y);
}

float3 tangent(in VertexData vData)
{
    return float3(vData.data1.z, vData.data1.w, vData.data2.x);
}

float2 tex_coord(in VertexData vData)
{
    return float2(vData.data2.y, vData.data2.z);
}

uint mat_id(in VertexData vData)
{
    return asuint(vData.data2.w);
}

#if defined(INDEX_BUFFER_BINDING)
uint3 primitive_indices(uint primitiveIndex)
{
    return uint3(_IndexBuffer[3 * primitiveIndex], _IndexBuffer[3 * primitiveIndex + 1], _IndexBuffer[3 * primitiveIndex + 2]);
}
#endif
#endif // MESH_UTILITIES_HLSL