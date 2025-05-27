/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "render_pipeline/gbuffer_renderer.h"
#include "tools/security.h"
#include "tools/shader_utils.h"
#include "tools/gpu_helpers.h"

GBufferRenderer::GBufferRenderer()
{
}

GBufferRenderer::~GBufferRenderer()
{
}

void GBufferRenderer::initialize(GraphicsDevice device, bool coopVectors)
{
    // Keep track of the device
    m_Device = device;

    // State tracking
    m_CoopVectors = coopVectors;

    // Initialize the samplers
    m_NearestSampler = graphics::resources::create_sampler(m_Device, SamplerDescriptor({ FilterMode::Point, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap, 1, 0.0, 0.0 }));
    m_LinearSampler = graphics::resources::create_sampler(m_Device, SamplerDescriptor({ FilterMode::Linear, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap, 1, 0.0, 15.0f}));
    m_AnisoSampler = graphics::resources::create_sampler(m_Device, SamplerDescriptor({ FilterMode::Anisotropic, SamplerMode::Wrap, SamplerMode::Wrap, SamplerMode::Wrap, 16, 0.0, 15.0f }));
}

void GBufferRenderer::release()
{
    // Sampler
    graphics::resources::destroy_sampler(m_NearestSampler);
    graphics::resources::destroy_sampler(m_LinearSampler);
    graphics::resources::destroy_sampler(m_AnisoSampler);

    // Compute shaders
    graphics::compute_shader::destroy_compute_shader(m_TextureCS);
    graphics::compute_shader::destroy_compute_shader(m_FMABC1CS);
    graphics::compute_shader::destroy_compute_shader(m_FMABC1_Repacked_CS);
    if (m_CoopVectors)
    {
        graphics::compute_shader::destroy_compute_shader(m_CVBC1CS);
        graphics::compute_shader::destroy_compute_shader(m_CVBC1_Repacked_CS);
    }
    graphics::compute_shader::destroy_compute_shader(m_DeferredLightingCS);
}

void GBufferRenderer::reload_shaders(const std::string& shaderLibrary, const std::vector<std::string>& shaderDefines)
{
    // Texture sampling
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\GBuffer\\Textures\\Inference.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_TextureCS);
    }

    // FMA Inference
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.defines.insert(csd.defines.end(), shaderDefines.begin(), shaderDefines.end());
        csd.filename = shaderLibrary + "\\GBuffer\\Inference.compute";
        csd.defines.push_back("LS_BC1_COMPRESSION");

        // BC1 version
        csd.kernelname = "main";
        compile_and_replace_compute_shader(m_Device, csd, m_FMABC1CS);

        csd.kernelname = "main_repacked";
        compile_and_replace_compute_shader(m_Device, csd, m_FMABC1_Repacked_CS);
    }

    // Coop vector inference
    if (m_CoopVectors)
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.defines.insert(csd.defines.end(), shaderDefines.begin(), shaderDefines.end());
        csd.filename = shaderLibrary + "\\GBuffer\\Inference.compute";
        csd.defines.push_back("LS_BC1_COMPRESSION");
        csd.defines.push_back("COOP_VECTOR_SUPPORTED");

        // BC1 version
        csd.kernelname = "main";
        compile_and_replace_compute_shader(m_Device, csd, m_CVBC1CS, true);

        // Repacked version
        csd.kernelname = "main_repacked";
        compile_and_replace_compute_shader(m_Device, csd, m_CVBC1_Repacked_CS, true);
    }

    // Deferred lighting
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Lighting\\Lit.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_DeferredLightingCS);
    }
}

void GBufferRenderer::evaluate_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, 
    GraphicsBuffer visibilityBuffer, GraphicsBuffer indexationBuffer, GraphicsBuffer indirectBuffer, GraphicsBuffer outputBuffer,
    const TextureSet& texSet, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, FilteringMode filteringMode)
{
    // CBVs
    graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_TextureCS, "_GlobalCB", globalCB);

    // Common buffers
    graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_TextureCS, "_VisibilityBuffer", visibilityBuffer);
    graphics::command_buffer::set_compute_shader_buffer(cmdB, m_TextureCS, "_TileBuffer", indexationBuffer);
    graphics::command_buffer::set_compute_shader_buffer(cmdB, m_TextureCS, "_VertexBuffer", vertexBuffer);
    graphics::command_buffer::set_compute_shader_buffer(cmdB, m_TextureCS, "_IndexBuffer", indexBuffer);

    // Texture materials
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TextureCS, "_Texture0", texSet.tex0);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TextureCS, "_Texture1", texSet.tex1);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TextureCS, "_Texture2", texSet.tex2);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TextureCS, "_Texture3", texSet.tex3);
    graphics::command_buffer::set_compute_shader_texture(cmdB, m_TextureCS, "_Texture4", texSet.tex4);

    // Sampler
    switch (filteringMode)
    {
        case FilteringMode::Nearest:
            graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TextureCS, "s_texture_sampler", m_NearestSampler);
        break;
        case FilteringMode::Linear:
            graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TextureCS, "s_texture_sampler", m_LinearSampler);
        break;
        case FilteringMode::Anisotropic:
            graphics::command_buffer::set_compute_shader_sampler(cmdB, m_TextureCS, "s_texture_sampler", m_AnisoSampler);
        break;
    }

    // Output buffer
    graphics::command_buffer::set_compute_shader_buffer(cmdB, m_TextureCS, "_OutputBufferRW", outputBuffer);

    // Dispatch + Barrier
    graphics::command_buffer::dispatch_indirect(cmdB, m_TextureCS, indirectBuffer);
    graphics::command_buffer::uav_barrier_buffer(cmdB, outputBuffer);
}

void GBufferRenderer::partial_inference(CommandBuffer cmdB, ComputeShader targetCS, uint32_t indirectOffset, GraphicsBuffer tileBuffer, ConstantBuffer globalCB, GraphicsBuffer visibilityBuffer, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, GraphicsBuffer outputBuffer,
    const TileClassifier& classifier, bool useCoopVectors, const TSNC& network, FilteringMode filteringMode)
{
    // Network buffers
    const GPUNetworkCompressed& gpuNwk = network.gpu_network();

    // If valid kernel
    if (targetCS != 0)
    {
        // Constant buffers
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, targetCS, "_GlobalCB", globalCB);

        // Common buffers
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, targetCS, "_VisibilityBuffer", visibilityBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_TileBuffer", tileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_VertexBuffer", vertexBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_IndexBuffer", indexBuffer);

        // Latent Space
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS0Texture", gpuNwk.tex0);
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS1Texture", gpuNwk.tex1);
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS2Texture", gpuNwk.tex2);
        graphics::command_buffer::set_compute_shader_texture(cmdB, targetCS, "_LS3Texture", gpuNwk.tex3);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_UVOffsetBuffer", network.uv_offset_buffer());

        // Sampler
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
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPWeight0Buffer", useCoopVectors ? gpuNwk.mlp.weight0OptimalBuffer : gpuNwk.mlp.weight0Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPBias0Buffer", gpuNwk.mlp.bias0Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPWeight1Buffer", useCoopVectors ? gpuNwk.mlp.weight1OptimalBuffer : gpuNwk.mlp.weight1Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPBias1Buffer", gpuNwk.mlp.bias1Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPWeight2Buffer", useCoopVectors ? gpuNwk.mlp.weight2OptimalBuffer : gpuNwk.mlp.weight2Buffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_MLPBias2Buffer", gpuNwk.mlp.bias2Buffer);

        // Output buffer
        graphics::command_buffer::set_compute_shader_buffer(cmdB, targetCS, "_OutputBufferRW", outputBuffer);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch_indirect(cmdB, targetCS, classifier.indirect_buffer(), indirectOffset);
        graphics::command_buffer::uav_barrier_buffer(cmdB, outputBuffer);
    }
}

void GBufferRenderer::evaluate_neural_cmp_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, GraphicsBuffer visibilityBuffer, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, GraphicsBuffer outputBuffer,
    const TileClassifier& classifier, bool useCoopVectors, const TSNC& network, FilteringMode filteringMode)
{
    // Uniform inference
    ComputeShader uniformCS = useCoopVectors ? m_CVBC1CS : m_FMABC1CS;
    graphics::command_buffer::start_section(cmdB, "Uniform inference");
    partial_inference(cmdB, uniformCS, 3 * sizeof(uint32_t), classifier.uniform_tiles_buffer(), globalCB, visibilityBuffer, vertexBuffer, indexBuffer, outputBuffer, classifier, useCoopVectors, network, filteringMode);
    graphics::command_buffer::end_section(cmdB);

    // Repacked inference
    ComputeShader repackedCS = useCoopVectors ? m_CVBC1_Repacked_CS : m_FMABC1_Repacked_CS;
    graphics::command_buffer::start_section(cmdB, "Repacked inference");
    partial_inference(cmdB, repackedCS, 9 * sizeof(uint32_t), classifier.repacked_tiles_buffer(), globalCB, visibilityBuffer, vertexBuffer, indexBuffer, outputBuffer, classifier, useCoopVectors, network, filteringMode);
    graphics::command_buffer::end_section(cmdB);
}

void GBufferRenderer::lighting_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, 
    GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl,
    GraphicsBuffer gbuffer, GraphicsBuffer tileBuffer, GraphicsBuffer indirectBuffer, RenderTexture visibilityBuffer, RenderTexture shadowTexture,
    RenderTexture colorTexture)
{
    graphics::command_buffer::start_section(cmdB, "Deferred Lighting");
    {
        // CBV
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_DeferredLightingCS, "_GlobalCB", globalCB);

        // Input buffers
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_DeferredLightingCS, "_VisibilityBuffer", visibilityBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_DeferredLightingCS, "_InferenceBuffer", gbuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_DeferredLightingCS, "_TileBuffer", tileBuffer);
        graphics::command_buffer::set_compute_shader_texture(cmdB, m_DeferredLightingCS, "_PreIntegratedFGDTexture", ibl.pre_integrated_fgd());
        graphics::command_buffer::set_compute_shader_texture(cmdB, m_DeferredLightingCS, "_ConvolvedIBLTexture", ibl.convolved_ggx_ibl());
        graphics::command_buffer::set_compute_shader_texture(cmdB, m_DeferredLightingCS, "_IndirectDiffuseTexture", ibl.convolved_lambert_ibl());
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_DeferredLightingCS, "_VertexBuffer", vertexBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_DeferredLightingCS, "_IndexBuffer", indexBuffer);
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_DeferredLightingCS, "_ShadowTexture", shadowTexture);

        // Output buffer
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_DeferredLightingCS, "_ColorTextureRW", colorTexture);

        // Samplers
        graphics::command_buffer::set_compute_shader_sampler(cmdB, m_DeferredLightingCS, "s_fgd_sampler", ibl.fgd_sampler());
        graphics::command_buffer::set_compute_shader_sampler(cmdB, m_DeferredLightingCS, "s_ggx_sampler", ibl.ggx_sampler());
        graphics::command_buffer::set_compute_shader_sampler(cmdB, m_DeferredLightingCS, "s_lambert_sampler", ibl.lambert_sampler());

        // Dispatch + Barrier
        graphics::command_buffer::dispatch_indirect(cmdB, m_DeferredLightingCS, indirectBuffer);
        graphics::command_buffer::uav_barrier_render_texture(cmdB, colorTexture);
    }
    graphics::command_buffer::end_section(cmdB);
}