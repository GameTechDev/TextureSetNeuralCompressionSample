/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef LIGHTLOOP_HLSL
#define LIGHTLOOP_HLSL

// Includes
#include "shader_lib/bsdf.hlsl"
#include "shader_lib/color.hlsl"

// SRVs
TextureCube<float4> _ConvolvedIBLTexture: register(CONVOLVED_GGX_BINDING);
TextureCube<float4> _IndirectDiffuseTexture: register(CONVOLVED_LAMBERT_BINDING);

// Samplers
sampler s_ggx_sampler: register(GGX_IBL_SAMPLER_BINDING);
sampler s_lambert_sampler: register(LAMBERT_IBL_SAMPLER_BINDING);

struct LightingResult
{
    float3 directDiff;
    float3 directSpec;
    float3 indirectDiff;
    float3 indirectSpec;
};

void sun_lighting(in BSDFData bsdfData, in GeometryData geometryData, in PreLightData prelightData, inout LightingResult lightingResult)
{
    eval_bsdf(bsdfData, geometryData, prelightData, _SunDirection, lightingResult.directDiff, lightingResult.directSpec);
}

void eval_prelight_data(in BSDFData bsdfData, float3 V, inout PreLightData preLightData)
{
    preLightData.NdotV = dot(bsdfData.normalWS, V);
    preLightData.clampedNdotV = clamp(preLightData.NdotV, 0.0, 1.0);

    // Used for IBLs
    float specularReflectivity;
    pre_integrated_FGD_GGX_disney_diffuse(preLightData.clampedNdotV, bsdfData.roughness, bsdfData.fresnel0, bsdfData.fresnel90, preLightData.specularFGD, preLightData.diffuseFGD, specularReflectivity);
    preLightData.energyCompensation = 1.0 / specularReflectivity - 1.0;
    preLightData.partLambdaV = get_smith_joint_ggx_part_lambdaV(preLightData.clampedNdotV, bsdfData.roughness);
}

void evaluate_bsdf_data(in SurfaceData surfaceData, in GeometryData geometryData, out BSDFData bsdfData)
{
    // Apply a gamma
    surfaceData.baseColor = pow(surfaceData.baseColor, 1.6);

    // Evaluate the bsdf data
    bsdfData.diffuseColor = compute_diffuse_color(surfaceData.baseColor, surfaceData.metalness);
    bsdfData.normalWS = combine_normals(geometryData.geoNormalWS, geometryData.tangentWS, surfaceData.normalTS);
    bsdfData.ambientOcclusion = surfaceData.ambientOcclusion;
    bsdfData.perceptualRoughness = surfaceData.perceptualRoughness;
    bsdfData.roughness = perceptual_roughness_to_roughness(surfaceData.perceptualRoughness);
    bsdfData.thickness = surfaceData.thickness;
    bsdfData.mask = surfaceData.mask;
    bsdfData.fresnel0 = compute_fresnel0(surfaceData.baseColor, surfaceData.metalness, DEFAULT_SPECULAR_VALUE);
    bsdfData.fresnel90 = compute_f90(bsdfData.fresnel0);
}

void environment_lighting(in BSDFData bsdfData, in PreLightData preLightData, GeometryData geometryData, inout LightingResult lightingResult)
{
    // Reflection vector
    float3 R = reflect(-geometryData.viewWS, bsdfData.normalWS);

    // Specular lobe
    float mipLevel = perceptual_roughness_to_mipmap_level(bsdfData.perceptualRoughness, 6.0);
    float3 preLD_S = _ConvolvedIBLTexture.SampleLevel(s_ggx_sampler, R, mipLevel).xyz;
    lightingResult.indirectSpec = preLightData.specularFGD * preLD_S.xyz * bsdfData.fresnel0;

    // Indirect specular    
    float3 preLD_D = _IndirectDiffuseTexture.SampleLevel(s_lambert_sampler, geometryData.geoNormalWS, 0.0).xyz;
    lightingResult.indirectDiff = preLightData.diffuseFGD * preLD_D * bsdfData.ambientOcclusion * bsdfData.diffuseColor;
}

float3 evaluate_lighting(in BSDFData bsdfData, in GeometryData geometryData, float shadow)
{
    // Initialize the lighting data
    LightingResult lightingResult;
    lightingResult.directDiff = (0.0);
    lightingResult.directSpec = (0.0);
    lightingResult.indirectDiff = (0.0);
    lightingResult.indirectSpec = (0.0);

    PreLightData preLightData;
    eval_prelight_data(bsdfData, geometryData.viewWS, preLightData);

    // Lighting sub-parts
    sun_lighting(bsdfData, geometryData, preLightData, lightingResult);
    environment_lighting(bsdfData, preLightData, geometryData, lightingResult);

    // Combine the lighting
    float3 finalLighting = (lightingResult.directDiff + lightingResult.directSpec) * shadow;
    finalLighting += (lightingResult.indirectDiff + lightingResult.indirectSpec) * INDIRECT_LIGHTING_MULTIPLIER;
        
    // Return the result
    return max(finalLighting, 0.0);
}

#endif // LIGHTLOOP_HLSL