/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"

#include "render_pipeline/material_renderer.h"

#include "tools/security.h"
#include "tools/shader_utils.h"
#include "tools/gpu_helpers.h"

MaterialRenderer::MaterialRenderer()
{
}

MaterialRenderer::~MaterialRenderer()
{
}

void MaterialRenderer::initialize(GraphicsDevice device, bool coopVectors)
{
    // Keep track of the device
    m_Device = device;

    // State tracking
    m_CoopVectors = coopVectors;

    // Create the samplers
    m_NearestSampler = graphics::resources::create_sampler(m_Device, SamplerDescriptor({ FilterMode::Point, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp, 1, 0.0, 0.0 }));
    m_LinearSampler = graphics::resources::create_sampler(m_Device, SamplerDescriptor({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp, 1, 0.0, 15.0f }));
    m_AnisoSampler = graphics::resources::create_sampler(m_Device, SamplerDescriptor({ FilterMode::Anisotropic, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp, 16, 0.0, 15.0f }));
}

void MaterialRenderer::release()
{
    // Destroy the sampler
    graphics::resources::destroy_sampler(m_NearestSampler);
    graphics::resources::destroy_sampler(m_LinearSampler);
    graphics::resources::destroy_sampler(m_AnisoSampler);

    graphics::compute_shader::destroy_compute_shader(m_TexturesCS);

    graphics::compute_shader::destroy_compute_shader(m_FMABC1CS);
    graphics::compute_shader::destroy_compute_shader(m_FMABC1_Repacked_CS);

    if (m_CoopVectors)
    {
        graphics::compute_shader::destroy_compute_shader(m_CVBC1CS);
        graphics::compute_shader::destroy_compute_shader(m_CVBC1_Repacked_CS);
    }
}

void MaterialRenderer::reload_shaders(const std::string& shaderLibrary, const TSNC& network)
{
    // Textures
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Material\\Textures\\MaterialPass.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_TexturesCS);
    }

    // FMA inference
    const std::vector<std::string>& shaderDefines = network.shader_defines();
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.defines.insert(csd.defines.end(), shaderDefines.begin(), shaderDefines.end());
        csd.filename = shaderLibrary + "\\Material\\MaterialPass.compute";
        csd.defines.push_back("LS_BC1_COMPRESSION");

        // BC1 version
        csd.kernelname = "main";
        compile_and_replace_compute_shader(m_Device, csd, m_FMABC1CS);

        // BC1 version Repacked
        csd.kernelname = "main_repacked";
        compile_and_replace_compute_shader(m_Device, csd, m_FMABC1_Repacked_CS);
    }

    // Coop vector inference
    if (m_CoopVectors)
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.defines.insert(csd.defines.end(), shaderDefines.begin(), shaderDefines.end());
        csd.filename = shaderLibrary + "\\Material\\MaterialPass.compute";
        csd.kernelname = "main";
        csd.defines.push_back("COOP_VECTOR_SUPPORTED");
        csd.defines.push_back("LS_BC1_COMPRESSION");

        // BC1 version
        csd.kernelname = "main";
        compile_and_replace_compute_shader(m_Device, csd, m_CVBC1CS, true);

        // BC1 version Repacked
        csd.kernelname = "main_repacked";
        compile_and_replace_compute_shader(m_Device, csd, m_CVBC1_Repacked_CS, true);
    }
}

void MaterialRenderer::evaluate_indirect(CommandBuffer cmdB, ConstantBuffer globalCB,
    GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl, const TextureSet& texSet, FilteringMode filteringMode,
    RenderTexture visilityBuffer, GraphicsBuffer shadowTexture, GraphicsBuffer indexationBuffer, GraphicsBuffer indirectBuffer, RenderTexture colorTexture)
{
    // Constant buffer
    graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_TexturesCS, "_GlobalCB", globalCB);

    // SRVs
    graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_TexturesCS, "_VisibilityBuffer", visilityBuffer);
    graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_TexturesCS, "_ShadowTexture", shadowTexture);
    graphics::command_buffer::set_compute_shader_buffer(cmdB, m_TexturesCS, "_TileBuffer", indexationBuffer);
    graphics::command_buffer::set_compute_shader_buffer(cmdB, m_TexturesCS, "_VertexBuffer", vertexBuffer);
    graphics::command_buffer::set_compute_shader_buffer(cmdB, m_TexturesCS, "_IndexBuffer", indexBuffer);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_PreIntegratedFGDTexture", ibl.pre_integrated_fgd());
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_ConvolvedIBLTexture", ibl.convolved_ggx_ibl());
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_IndirectDiffuseTexture", ibl.convolved_lambert_ibl());

    // Material texture
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_Texture0", texSet.tex0);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_Texture1", texSet.tex1);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_Texture2", texSet.tex2);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_Texture3", texSet.tex3);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TexturesCS, "_Texture4", texSet.tex4);

    // Samplers
    graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TexturesCS, "s_fgd_sampler", ibl.fgd_sampler());
    graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TexturesCS, "s_ggx_sampler", ibl.ggx_sampler());
    graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TexturesCS, "s_lambert_sampler", ibl.lambert_sampler());
    switch (filteringMode)
    {
        case FilteringMode::Nearest:
            graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TexturesCS, "s_texture_sampler", m_NearestSampler);
            break;
        case FilteringMode::Linear:
            graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TexturesCS, "s_texture_sampler", m_LinearSampler);
            break;
        case FilteringMode::Anisotropic:
            graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TexturesCS, "s_texture_sampler", m_AnisoSampler);
            break;
    }

    // Output buffer
    graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_TexturesCS, "_ColorTextureRW", colorTexture);

    // Dispatch + barrier
    graphics::command_buffer::dispatch_indirect(cmdB, m_TexturesCS, indirectBuffer);
    graphics::command_buffer::uav_barrier_render_texture(cmdB, colorTexture);
}

void MaterialRenderer::partial_inference(CommandBuffer cmdB, ComputeShader targetCS, uint32_t indirectOffset, GraphicsBuffer tileBuffer, ConstantBuffer globalCB,
    const TSNC& network, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl, bool useCooperativeVectors, FilteringMode filteringMode,
    RenderTexture visilityBuffer, GraphicsBuffer shadowTexture, const TileClassifier& classifier, RenderTexture colorTexture)
{
    const GPUNetworkCompressed& gpuNwk = network.gpu_network();

    // Is there a valid kernel to run?
    if (targetCS != 0)
    {
        // CBVs
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, targetCS, "_GlobalCB", globalCB);

        // Input buffers
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, targetCS, "_VisibilityBuffer", visilityBuffer);
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, targetCS, "_ShadowTexture", shadowTexture);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_TileBuffer", tileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_VertexBuffer", vertexBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_IndexBuffer", indexBuffer);
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_PreIntegratedFGDTexture", ibl.pre_integrated_fgd());
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_ConvolvedIBLTexture", ibl.convolved_ggx_ibl());
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_IndirectDiffuseTexture", ibl.convolved_lambert_ibl());

        // Samplers
        graphics::command_buffer::set_compute_shader_sampler(cmdB, targetCS, "s_fgd_sampler", ibl.fgd_sampler());
        graphics::command_buffer::set_compute_shader_sampler(cmdB, targetCS, "s_ggx_sampler", ibl.ggx_sampler());
        graphics::command_buffer::set_compute_shader_sampler(cmdB, targetCS, "s_lambert_sampler", ibl.lambert_sampler());

        // Output buffer
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, targetCS, "_ColorTextureRW", colorTexture);

        // Latent Space
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS0Texture", gpuNwk.tex0);
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS1Texture", gpuNwk.tex1);
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS2Texture", gpuNwk.tex2);
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS3Texture", gpuNwk.tex3);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_UVOffsetBuffer", network.uv_offset_buffer());

        // Samplers
        switch (filteringMode)
        {
            case FilteringMode::Nearest:
                graphics::command_buffer::set_compute_shader_sampler(cmdB, targetCS, "bc1_linear_clamp_sampler", m_NearestSampler);
                break;
            case FilteringMode::Linear:
                graphics::command_buffer::set_compute_shader_sampler(cmdB, targetCS, "bc1_linear_clamp_sampler", m_LinearSampler);
                break;
            case FilteringMode::Anisotropic:
                graphics::command_buffer::set_compute_shader_sampler(cmdB, targetCS, "bc1_linear_clamp_sampler", m_AnisoSampler);
                break;
        }

        // MLPs
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPWeight0Buffer", useCooperativeVectors ? gpuNwk.mlp.weight0OptimalBuffer : gpuNwk.mlp.weight0Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPBias0Buffer", gpuNwk.mlp.bias0Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPWeight1Buffer", useCooperativeVectors ? gpuNwk.mlp.weight1OptimalBuffer : gpuNwk.mlp.weight1Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPBias1Buffer", gpuNwk.mlp.bias1Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPWeight2Buffer", useCooperativeVectors ? gpuNwk.mlp.weight2OptimalBuffer : gpuNwk.mlp.weight2Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPBias2Buffer", gpuNwk.mlp.bias2Buffer);

        // Dispatch + barrier
        graphics::command_buffer::dispatch_indirect(cmdB, targetCS, classifier.indirect_buffer(), indirectOffset);
        graphics::command_buffer::uav_barrier_render_texture(cmdB, colorTexture);
    }
}

void MaterialRenderer::evaluate_neural_cmp_indirect(CommandBuffer cmdB, ConstantBuffer globalCB,
        const TSNC& network, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl, bool useCooperativeVectors, FilteringMode filteringMode,
        RenderTexture visilityBuffer, GraphicsBuffer shadowTexture, const TileClassifier& classifier, RenderTexture colorTexture)
{
    // Pick the right kernel
    ComputeShader uniformTileCS = useCooperativeVectors ? m_CVBC1CS : m_FMABC1CS;
    graphics::command_buffer::start_section(cmdB, "Uniform inference");
    partial_inference(cmdB, uniformTileCS, 3 * sizeof(uint32_t), classifier.uniform_tiles_buffer(), globalCB, network, vertexBuffer, indexBuffer, ibl, useCooperativeVectors, filteringMode, visilityBuffer, shadowTexture, classifier, colorTexture);
    graphics::command_buffer::end_section(cmdB);


    // Pick the right kernel
    ComputeShader repackedTilesCS = useCooperativeVectors ? m_CVBC1_Repacked_CS : m_FMABC1_Repacked_CS;
    graphics::command_buffer::start_section(cmdB, "Repacked inference");
    partial_inference(cmdB, repackedTilesCS, 9 * sizeof(uint32_t), classifier.repacked_tiles_buffer(), globalCB, network, vertexBuffer, indexBuffer, ibl, useCooperativeVectors, filteringMode, visilityBuffer, shadowTexture, classifier, colorTexture);
    graphics::command_buffer::end_section(cmdB);
}