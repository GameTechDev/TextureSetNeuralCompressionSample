/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "graphics/descriptors.h"
#include "tools/texture_utils.h"

class IBL
{
public:
    IBL();
    ~IBL();

    // Initialize and release
    void initialize(GraphicsDevice device, const std::string& textureLibrary);
    void release();

    // Reload the shaders
    void reload_shaders(const std::string& shaderLibrary);

    // Upload the texture
    void upload_textures(CommandQueue cmdQ, CommandBuffer cmdB);

    // Render the cubemap to the currently bound render target
    void render_cubemap(CommandBuffer cmd, ConstantBuffer globalCB, RenderTexture colorTexture, RenderTexture shadowTexture, GraphicsBuffer displacementBuffer);

    // Return the texture if needed
    Texture pre_integrated_fgd() const { return m_FGDTexture; }
    Sampler fgd_sampler() const { return m_FGDSampler; }

    // GGX Convolve
    Texture convolved_ggx_ibl() const { return m_ConvolvedGGXTexture; }
    Sampler ggx_sampler() const { return m_GGXSampler; }

    // Lambert Convolve
    Texture convolved_lambert_ibl() const { return m_ConvolvedLambertTexture; }
    Sampler lambert_sampler() const { return m_LambertSampler; }

private:
    // Graphics device
    GraphicsDevice m_Device = 0;

    // CPU Data
    BinaryTexture m_FGDData = BinaryTexture();
    BinaryTexture m_ConvolvedGGXData = BinaryTexture();
    BinaryTexture m_ConvolvedLambertData = BinaryTexture();
    BinaryTexture m_BackgroundData = BinaryTexture();

    // FGD Karis convolution
    Texture m_FGDTexture = 0;
    Sampler m_FGDSampler = {};

    // GGX Convolve
    Texture m_ConvolvedGGXTexture = 0;
    Sampler m_GGXSampler = {};

    // Lambert convolve
    Texture m_ConvolvedLambertTexture = 0;
    Sampler m_LambertSampler = {};

    // Background texture
    Texture m_BackgroundTexture = 0;

    // Shader
    GraphicsPipeline m_CubemapGP = 0;
    GraphicsPipeline m_CubemapSimpleGP = 0;
};
