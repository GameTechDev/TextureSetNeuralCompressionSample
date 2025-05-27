/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef visibility_utilities_HLSL
#define visibility_utilities_HLSL

// Includes
#include "shader_lib/transform.hlsl"

struct BarycentricDeriv
{
  float3 bary;
  float3 dx;
  float3 dy;
};

BarycentricDeriv calc_full_bary(float4 pt0, float4 pt1, float4 pt2, float2 pixelNdc, float2 winSize)
{
  BarycentricDeriv ret = (BarycentricDeriv)0;

  float3 invW = rcp(float3(pt0.w, pt1.w, pt2.w));

  float2 ndc0 = pt0.xy * invW.x;
  float2 ndc1 = pt1.xy * invW.y;
  float2 ndc2 = pt2.xy * invW.z;

  float invDet = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));
  ret.dx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
  ret.dy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;
  float ddxSum = dot(ret.dx, float3(1,1,1));
  float ddySum = dot(ret.dy, float3(1,1,1));

  float2 deltaVec = pixelNdc - ndc0;
  float interpInvW = invW.x + deltaVec.x*ddxSum + deltaVec.y*ddySum;
  float interpW = rcp(interpInvW);

  ret.bary.x = interpW * (invW[0] + deltaVec.x*ret.dx.x + deltaVec.y*ret.dy.x);
  ret.bary.y = interpW * (0.0f    + deltaVec.x*ret.dx.y + deltaVec.y*ret.dy.y);
  ret.bary.z = interpW * (0.0f    + deltaVec.x*ret.dx.z + deltaVec.y*ret.dy.z);

  ret.dx *= (2.0f/winSize.x);
  ret.dy *= (2.0f/winSize.y);
  ddxSum    *= (2.0f/winSize.x);
  ddySum    *= (2.0f/winSize.y);

  ret.dy *= -1.0f;
  ddySum    *= -1.0f;

  float interpW_ddx = 1.0f / (interpInvW + ddxSum);
  float interpW_ddy = 1.0f / (interpInvW + ddySum);

  ret.dx = interpW_ddx*(ret.bary*interpInvW + ret.dx) - ret.bary;
  ret.dy = interpW_ddy*(ret.bary*interpInvW + ret.dy) - ret.bary;  

  return ret;
}

BarycentricDeriv calc_full_bary_ws(float3 p0, float3 p1, float3 p2, float2 ndc)
{
	// Evluate the homogenous positions
	float4 p0NDC = evaluate_homogenous_position(p0 - _CameraPosition, _ViewProjectionMatrix);
	float4 p1NDC = evaluate_homogenous_position(p1 - _CameraPosition, _ViewProjectionMatrix);
	float4 p2NDC = evaluate_homogenous_position(p2 - _CameraPosition, _ViewProjectionMatrix);
	return calc_full_bary(p0NDC, p1NDC, p2NDC, ndc, float2(_ScreenSize));
}

float3 interpolate_with_deriv(BarycentricDeriv deriv, float v0, float v1, float v2)
{
  float3 mergedV = float3(v0, v1, v2);
  float3 ret;
  ret.x = dot(mergedV, deriv.bary);
  ret.y = dot(mergedV, deriv.dx);
  ret.z = dot(mergedV, deriv.dy);
  return ret;
}

void interpolate_with_deriv(BarycentricDeriv deriv, float2 v0, float2 v1, float2 v2, out float2 outVal, out float2 valDX, out float2 valDY)
{
  float3 outV0 = interpolate_with_deriv(deriv, v0.x, v1.x, v2.x);
  float3 outV1 = interpolate_with_deriv(deriv, v0.y, v1.y, v2.y);

  // Repack
  outVal = float2(outV0.x, outV1.x);
  valDX = float2(outV0.y, outV1.y);
  valDY = float2(outV0.z, outV1.z);
}

bool is_valid_visibility_value(uint visibilityData)
{
    return (visibilityData & 0x80000000) != 0;
}

uint pack_visibility_buffer(uint primtiveID)
{
    return (primtiveID & 0x7FFFFFFF) | 0x80000000;
}

bool unpack_visibility_buffer(uint visibilityData, out uint primtiveID)
{
    // Primitive ID
    primtiveID = visibilityData & 0x7FFFFFFF;

    // Validity
    return is_valid_visibility_value(visibilityData);
}

BarycentricDeriv evaluate_barycentrics(float3 p0, float3 p1, float3 p2, uint2 pixelCoords)
{
    // Compute the barycentrics and their derivatives
    float2 ndc = evaluate_ndc_coordinates(pixelCoords, _ScreenSize.xy);
    return calc_full_bary_ws(p0, p1, p2, ndc);
}

float3 evaluate_barycentrics_no_deriv(float3 p0, float3 p1, float3 p2, uint2 pixelCoords)
{
    // Compute the barycentrics and their derivatives
    float2 ndc = evaluate_ndc_coordinates(pixelCoords, _ScreenSize.xy);
    return calc_full_bary_ws(p0, p1, p2, ndc).bary;
}

float compute_lod(in float2 uv, in float2 uvDX, in float2 uvDY)
{
    float lodLevel = min(log2(max(length(uvDX * _TextureSize), length(uvDY * _TextureSize))), _EnableFiltering); 
    return clamp(lodLevel / _NumTextureLOD.x, 0.0, 1.0);
}
#endif // visibility_utilities_HLSL