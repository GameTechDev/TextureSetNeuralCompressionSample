/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef BSDF_HLSL
#define BSDF_HLSL

// Requirement
#include "shader_lib/transform.hlsl"
#include "shader_lib/pbr.hlsl"
#include "shader_lib/material.hlsl"

// Fixed resolution
#define FGDTEXTURE_RESOLUTION 64

// SRVs
Texture2D<float4> _PreIntegratedFGDTexture: register(PRE_INTEGRATED_FGD_BINDING);

// Samplers
sampler s_fgd_sampler: register(FGD_SAMPLER_BINDING);

// 30 degrees
#define TRANSMISSION_WRAP_ANGLE (PI/6)
#define TRANSMISSION_WRAP_LIGHT cos(PI/2 - TRANSMISSION_WRAP_ANGLE)
#define DEFAULT_SPECULAR_VALUE 0.08

struct BSDFData
{
    float3 diffuseColor;
    float3 normalWS;
    float ambientOcclusion;
    float roughness;
    float perceptualRoughness;
    float thickness;
    float2 mask;
    float3 fresnel0;
    float fresnel90;
};

struct PreLightData
{
    float NdotV;
    float clampedNdotV;
    float energyCompensation;
    float3 specularFGD;
    float diffuseFGD;
    float partLambdaV;
};

float2 remap01_to_half_texel_coord(float2 coord, float2 size)
{
    const float2 invSize = 1.0f / size;
    const float2 start = 0.5 * invSize;
    const float2 len   = 1 - invSize;
    return coord * len + start;
}

void pre_integrated_FGD_GGX_disney_diffuse(float NdotV, float perceptualRoughness, float3 fresnel0, float F90,
        out float3 specularFGD, out float diffuseFGD, out float reflectivity)
{
    float2 coordLUT = remap01_to_half_texel_coord(float2(sqrt(NdotV), perceptualRoughness), float2(FGDTEXTURE_RESOLUTION, FGDTEXTURE_RESOLUTION));
    float3 preFGD = _PreIntegratedFGDTexture.Sample(s_fgd_sampler, coordLUT).xyz;
    specularFGD = (F90 - fresnel0) * preFGD.xxx + fresnel0 * preFGD.yyy;
    diffuseFGD = preFGD.z + 0.5;
    reflectivity = preFGD.y;
}

void get_bsdf_angle(float3 V, float3 L, float NdotL, float NdotV,
                  out float LdotV, out float NdotH, out float LdotH, out float invLenLV)
{
    LdotV = dot(L, V);
    invLenLV = 1.0 / sqrt(max(2.0 * LdotV + 2.0, FLT_EPSILON));
    NdotH = clamp((NdotL + NdotV) * invLenLV, 0.0, 1.0);
    LdotH = clamp(invLenLV * LdotV + invLenLV, 0.0, 1.0);
}

float get_smith_joint_ggx_part_lambdaV(float NdotV, float roughness)
{
    float a2 = Sq(roughness);
    return sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
}

float dv_smith_joint_ggx(float NdotH, float NdotL, float NdotV, float roughness, float partLambdaV)
{
    float a2 = Sq(roughness);
    float s = (NdotH * a2 - NdotH) * NdotH + 1.0;
    float lambdaV = NdotL * partLambdaV;
    float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    float2 D = float2(a2, s * s);
    float2 G = float2(1, lambdaV + lambdaL);
    return INV_PI * 0.5 * (D.x * G.x) / max(D.y * G.y, 1e-6);
}

float disney_diffuse_no_pi(float NdotV, float NdotL, float LdotV, float perceptualRoughness)
{
    float fd90 = 0.5 + (perceptualRoughness + perceptualRoughness * LdotV);
    float lightScatter = f_schlick(1.0, fd90, NdotL);
    float viewScatter = f_schlick(1.0, fd90, NdotV);
    return 1.0 / (1.03571) * (lightScatter * viewScatter);
}

float disney_diffuse(float NdotV, float NdotL, float LdotV, float perceptualRoughness)
{
    return INV_PI * disney_diffuse_no_pi(NdotV, NdotL, LdotV, perceptualRoughness);
}

void eval_bsdf(in BSDFData bsdfData, GeometryData geometryData, inout PreLightData preLightData, float3 L, inout float3 diffuseLighting, inout float3 specLighting)
{
    // Base ters
    float3 N = bsdfData.normalWS;
    float NdotV = preLightData.NdotV;
    float NdotL = dot(N, L);
    float clampedNdotL = clamp(NdotL, 0.0, 1.0);
    float flippedNdotL = clamp(compute_wrapped_diffuse_lighting(dot(geometryData.geoNormalWS, L), TRANSMISSION_WRAP_LIGHT), 0.0, 1.0);
    float diffuseNdotL = clampedNdotL;
    float LdotV, NdotH, LdotH, invLenLV;
    get_bsdf_angle(geometryData.viewWS, L, NdotL, NdotV, LdotV, NdotH, LdotH, invLenLV);

    // BRDF parts
    float3 F = f_schlick(bsdfData.fresnel0, bsdfData.fresnel90, LdotH);
    float DV = dv_smith_joint_ggx(NdotH, abs(NdotL), preLightData.clampedNdotV, bsdfData.roughness, preLightData.partLambdaV);

    // Lighting terms
    float3 specTerm = F * DV;
    float diffTerm = disney_diffuse(preLightData.clampedNdotV, abs(NdotL), LdotV, bsdfData.perceptualRoughness);
    float diffR = diffTerm * diffuseNdotL;
    float diffT = diffTerm * flippedNdotL * bsdfData.mask.y * 0.2;
    float3 specR = specTerm * clampedNdotL;

    // Combine and return
    diffuseLighting += diffR * bsdfData.diffuseColor + diffT * SSS_COLOR;
    specLighting += specR;
}

#endif // BSDF_HLSL