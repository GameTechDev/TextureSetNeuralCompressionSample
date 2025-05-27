/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "render_pipeline/ibl.h"
#include "tools/texture_utils.h"
#include "tools/shader_utils.h"
#include "tools/security.h"

IBL::IBL()
{
}

IBL::~IBL()
{
}

void IBL::initialize(GraphicsDevice device, const std::string& textureLibrary)
{
    // Keep track of the device
    m_Device = device;

    // Pre-convolved FGD
    {
        // Read the fdg data
        const std::string& fgdPath = textureLibrary + "\\pre_integrated_fdg.tex_bin";
        binary_texture::import_binary_texture(fgdPath.c_str(), m_FGDData);

        // Allocate the texture
        TextureDescriptor desc;
        desc.type = m_FGDData.type;
        desc.width = m_FGDData.width;
        desc.height = m_FGDData.height;
        desc.depth = m_FGDData.depth;
        desc.mipCount = m_FGDData.mipCount;
        desc.format = m_FGDData.format;
        m_FGDTexture = graphics::resources::create_texture(device, desc);

        // Create the sampler
        SamplerDescriptor samplerDesc = SamplerDescriptor({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp, 0, 0.0f, 0.0f });
        m_FGDSampler = graphics::resources::create_sampler(m_Device, samplerDesc);
    }

    // Pre-convolved GGX Cubemap
    {
        // Read the data
        const std::string& convolvedPath = textureLibrary + "\\convolved_ibl_ggx.tex_bin";
        binary_texture::import_binary_texture(convolvedPath.c_str(), m_ConvolvedGGXData);

        // Allocate the texture
        TextureDescriptor desc;
        desc.type = m_ConvolvedGGXData.type;
        desc.width = m_ConvolvedGGXData.width;
        desc.height = m_ConvolvedGGXData.height;
        desc.depth = m_ConvolvedGGXData.depth;
        desc.mipCount = m_ConvolvedGGXData.mipCount;
        desc.format = m_ConvolvedGGXData.format;
        m_ConvolvedGGXTexture = graphics::resources::create_texture(device, desc);

        // Create the sampler
        SamplerDescriptor samplerDesc = SamplerDescriptor({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp, 0, 0.0f, (float)desc.mipCount - 1.0f});
        m_GGXSampler = graphics::resources::create_sampler(m_Device, samplerDesc);
    }

    // Pre-convolved Lambert Cubemap
    {
        // Read the data
        const std::string& convolvedPath = textureLibrary + "\\convolved_ibl_lambert.tex_bin";
        binary_texture::import_binary_texture(convolvedPath.c_str(), m_ConvolvedLambertData);

        // Allocate the texture
        TextureDescriptor desc;
        desc.type = m_ConvolvedLambertData.type;
        desc.width = m_ConvolvedLambertData.width;
        desc.height = m_ConvolvedLambertData.height;
        desc.depth = m_ConvolvedLambertData.depth;
        desc.mipCount = m_ConvolvedLambertData.mipCount;
        desc.format = m_ConvolvedLambertData.format;
        m_ConvolvedLambertTexture = graphics::resources::create_texture(device, desc);

        // Create the sampler
        SamplerDescriptor samplerDesc = SamplerDescriptor({ FilterMode::Linear, SamplerMode::Clamp, SamplerMode::Clamp, SamplerMode::Clamp, 0, 0.0f, (float)desc.mipCount - 1.0f });
        m_LambertSampler = graphics::resources::create_sampler(m_Device, samplerDesc);
    }

    // Background texture
    {
        // Read the data
        const std::string& convolvedPath = textureLibrary + "\\convolved_ibl_bg.tex_bin";
        binary_texture::import_binary_texture(convolvedPath.c_str(), m_BackgroundData);

        // Allocate the texture
        TextureDescriptor desc;
        desc.type = m_BackgroundData.type;
        desc.width = m_BackgroundData.width;
        desc.height = m_BackgroundData.height;
        desc.depth = m_BackgroundData.depth;
        desc.mipCount = m_BackgroundData.mipCount;
        desc.format = m_BackgroundData.format;
        m_BackgroundTexture = graphics::resources::create_texture(device, desc);
    }
}

void IBL::release()
{
    // Shaders
    graphics::graphics_pipeline::destroy_graphics_pipeline(m_CubemapGP);

    // Textures
    graphics::resources::destroy_texture(m_FGDTexture);
    graphics::resources::destroy_texture(m_ConvolvedGGXTexture);
    graphics::resources::destroy_texture(m_ConvolvedLambertTexture);
    graphics::resources::destroy_texture(m_BackgroundTexture);

    // Samplers
    graphics::resources::destroy_sampler(m_FGDSampler);
    graphics::resources::destroy_sampler(m_GGXSampler);
    graphics::resources::destroy_sampler(m_LambertSampler);
}

void IBL::upload_textures(CommandQueue cmdQ, CommandBuffer cmdB)
{
    // FGD
    {
        GraphicsBuffer imageBuffer = graphics::resources::create_graphics_buffer(m_Device, m_FGDData.width * m_FGDData.height * sizeof(half4), sizeof(half4), GraphicsBufferType::Upload);
        graphics::resources::set_buffer_data(imageBuffer, (const char*)m_FGDData.data.data(), m_FGDData.width * m_FGDData.height * sizeof(half4));

        // Copy the buffer to a texture
        graphics::command_buffer::reset(cmdB);
        graphics::command_buffer::copy_buffer_into_texture(cmdB, imageBuffer, 0, m_FGDTexture, 0, 0);
        graphics::command_buffer::close(cmdB);
        graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
        graphics::command_queue::flush(cmdQ);

        // Destroy the graphics buffer
        graphics::resources::destroy_graphics_buffer(imageBuffer);
    }

    // Convolved map GGX
    {
        GraphicsBuffer imageBuffer = graphics::resources::create_graphics_buffer(m_Device, m_ConvolvedGGXData.data.size(), sizeof(half4), GraphicsBufferType::Upload);
        graphics::resources::set_buffer_data(imageBuffer, (const char*)m_ConvolvedGGXData.data.data(), m_ConvolvedGGXData.data.size());

        // Copy the buffer to a texture
        graphics::command_buffer::reset(cmdB);

        uint64_t offset = 0;
        uint64_t currentRes = m_ConvolvedGGXData.width;
        for (uint32_t mipIdx = 0; mipIdx < 7; ++mipIdx)
        {
            for (uint32_t faceIdx = 0; faceIdx < 6; ++faceIdx)
            {
                graphics::command_buffer::copy_buffer_into_texture(cmdB, imageBuffer, offset, m_ConvolvedGGXTexture, faceIdx, mipIdx);
                offset += currentRes * currentRes * sizeof(half4);
            }
            currentRes >>= 1;
        }
        graphics::command_buffer::close(cmdB);
        graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
        graphics::command_queue::flush(cmdQ);

        // Destroy the graphics buffer
        graphics::resources::destroy_graphics_buffer(imageBuffer);
    }

    // Convolved map Lambert
    {
        GraphicsBuffer imageBuffer = graphics::resources::create_graphics_buffer(m_Device, m_ConvolvedLambertData.data.size(), sizeof(half4), GraphicsBufferType::Upload);
        graphics::resources::set_buffer_data(imageBuffer, (const char*)m_ConvolvedLambertData.data.data(), m_ConvolvedLambertData.data.size());

        // Copy the buffer to a texture
        graphics::command_buffer::reset(cmdB);

        uint64_t offset = 0;
        uint64_t currentRes = m_ConvolvedLambertData.width;
        for (uint32_t faceIdx = 0; faceIdx < 6; ++faceIdx)
        {
            graphics::command_buffer::copy_buffer_into_texture(cmdB, imageBuffer, offset, m_ConvolvedLambertTexture, faceIdx, 0);
            offset += currentRes * currentRes * sizeof(half4);
        }
        graphics::command_buffer::close(cmdB);
        graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
        graphics::command_queue::flush(cmdQ);

        // Destroy the graphics buffer
        graphics::resources::destroy_graphics_buffer(imageBuffer);
    }

    // Background texture
    {
        GraphicsBuffer imageBuffer = graphics::resources::create_graphics_buffer(m_Device, m_BackgroundData.data.size(), sizeof(half4), GraphicsBufferType::Upload);
        graphics::resources::set_buffer_data(imageBuffer, (const char*)m_BackgroundData.data.data(), m_BackgroundData.data.size());

        // Copy the buffer to a texture
        graphics::command_buffer::reset(cmdB);

        uint64_t offset = 0;
        uint64_t currentRes = m_BackgroundData.width;
        for (uint32_t faceIdx = 0; faceIdx < 6; ++faceIdx)
        {
            graphics::command_buffer::copy_buffer_into_texture(cmdB, imageBuffer, offset, m_BackgroundTexture, faceIdx, 0);
            offset += currentRes * currentRes * sizeof(half4);
        }
        graphics::command_buffer::close(cmdB);
        graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
        graphics::command_queue::flush(cmdQ);

        // Destroy the graphics buffer
        graphics::resources::destroy_graphics_buffer(imageBuffer);
    }
}

void IBL::reload_shaders(const std::string& shaderLibrary)
{
    // Cubemap rendering
    {
        GraphicsPipelineDescriptor gpd;
        gpd.filename = shaderLibrary + "\\Cubemap.graphics";
        gpd.includeDirectories.push_back(shaderLibrary);
        gpd.isProcedural = true;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_CubemapGP);
    }
}

void IBL::render_cubemap(CommandBuffer cmdB, ConstantBuffer globalCB, RenderTexture colorTexture, RenderTexture shadowTexture, GraphicsBuffer displacementBuffer)
{
    // Render target
    graphics::command_buffer::set_render_texture(cmdB, colorTexture);

    // Constant buffer
    graphics::command_buffer::set_graphics_pipeline_cbuffer(cmdB, m_CubemapGP, "_GlobalCB", globalCB);

    // Input data
    graphics::command_buffer::set_graphics_pipeline_texture(cmdB, m_CubemapGP, "_BackgroundTexture", m_BackgroundTexture);
    graphics::command_buffer::set_graphics_pipeline_texture(cmdB, m_CubemapGP, "_IndirectDiffuseTexture", m_ConvolvedLambertTexture);
    graphics::command_buffer::set_graphics_pipeline_render_texture(cmdB, m_CubemapGP, "_ShadowTexture", shadowTexture);
    graphics::command_buffer::set_graphics_pipeline_buffer(cmdB, m_CubemapGP, "_DisplacementBuffer", displacementBuffer);

    // Sampler
    graphics::command_buffer::set_graphics_pipeline_sampler(cmdB, m_CubemapGP, "sampler_linear_clamp", m_LambertSampler);

    // Draw
    graphics::command_buffer::draw_procedural(cmdB, m_CubemapGP, 1, 1);
}
