/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "network/tsnc.h"
#include "math/operators.h"

#include "tools/directory_utilities.h"
#include "tools/gpu_helpers.h"
#include "tools/security.h"
#include "tools/shader_utils.h"
#include "tools/stream.h"
#include "tools/texture_utils.h"

TSNC::TSNC()
{
}

TSNC::~TSNC()
{
}

void TSNC::initialize(GraphicsDevice device, bool cvs)
{
    // Keep track of the device
    m_Device = device;
    m_CVS = cvs;
}

void TSNC::release()
{
    // Latent space
    graphics::resources::destroy_texture(m_Nwk.tex0);
    graphics::resources::destroy_texture(m_Nwk.tex1);
    graphics::resources::destroy_texture(m_Nwk.tex2);
    graphics::resources::destroy_texture(m_Nwk.tex3);
    graphics::resources::destroy_graphics_buffer(m_UVOffsetBuffer);
    
    // MLP
    mlp::destroy_gpu_mlp(m_Nwk.mlp);

    // Shaders
    graphics::compute_shader::destroy_compute_shader(m_FP32toFP16CS);
}

void TSNC::reload_network(const std::string& modelDir, uint32_t numSets)
{
    // Load the bc1 textures
    m_NumSets = numSets;
    m_TexData.resize(4 * numSets);
    m_UVOffset.resize(4 * numSets);
    m_MLPArray.resize(numSets);

    // Copy all the sets
    for (uint32_t setIdx = 0; setIdx < numSets; ++setIdx)
    {
        // Read the file to a buffer
        std::vector<char> mlpBuffer;
        load_file_to_array((modelDir + "\\mlp_" + std::to_string(setIdx) +".bin").c_str(), mlpBuffer);

        // Unpack the MLP and adjust
        const char* rawData = (const char*)mlpBuffer.data();
        unpack_type(rawData, m_MLPArray[setIdx]);
        mlp::align_dimensions(m_MLPArray[setIdx]);

        // Load the textures
        m_TexData[4 * setIdx + 0].texBuffer = load_bc1_to_graphics_buffer(m_Device, (modelDir + "\\tex0_" + std::to_string(setIdx) + ".bc1").c_str(), m_TexData[4 * setIdx + 0].texSize, m_UVOffset[4 * setIdx + 0]);
        m_TexData[4 * setIdx + 1].texBuffer = load_bc1_to_graphics_buffer(m_Device, (modelDir + "\\tex1_" + std::to_string(setIdx) + ".bc1").c_str(), m_TexData[4 * setIdx + 1].texSize, m_UVOffset[4 * setIdx + 1]);
        m_TexData[4 * setIdx + 2].texBuffer = load_bc1_to_graphics_buffer(m_Device, (modelDir + "\\tex2_" + std::to_string(setIdx) + ".bc1").c_str(), m_TexData[4 * setIdx + 2].texSize, m_UVOffset[4 * setIdx + 2]);
        m_TexData[4 * setIdx + 3].texBuffer = load_bc1_to_graphics_buffer(m_Device, (modelDir + "\\tex3_" + std::to_string(setIdx) + ".bc1").c_str(), m_TexData[4 * setIdx + 3].texSize, m_UVOffset[4 * setIdx + 3]);
    }

    // Create our Latent space runtime textures
    TextureDescriptor texDesc;
    texDesc.type = TextureType::Tex2DArray;
    texDesc.depth = numSets;
    texDesc.format = TextureFormat::BC1_RGB;
    texDesc.isUAV = false;

    texDesc.width = m_TexData[0].texSize.x;
    texDesc.height = m_TexData[0].texSize.y;
    texDesc.mipCount = m_TexData[0].texSize.z;
    m_Nwk.tex0 = graphics::resources::create_texture(m_Device, texDesc);

    texDesc.width = m_TexData[1].texSize.x;
    texDesc.height = m_TexData[1].texSize.y;
    texDesc.mipCount = m_TexData[1].texSize.z;
    m_Nwk.tex1 = graphics::resources::create_texture(m_Device, texDesc);

    texDesc.width = m_TexData[2].texSize.x;
    texDesc.height = m_TexData[2].texSize.y;
    texDesc.mipCount = m_TexData[2].texSize.z;
    m_Nwk.tex2 = graphics::resources::create_texture(m_Device, texDesc);

    texDesc.width = m_TexData[3].texSize.x;
    texDesc.height = m_TexData[3].texSize.y;
    texDesc.mipCount = m_TexData[3].texSize.z;
    m_Nwk.tex3 = graphics::resources::create_texture(m_Device, texDesc);

    // Allocate the MLP n the GPU
    mlp::allocate_gpu_mlp_array(m_Device, m_MLPArray, m_Nwk.mlp);

    // Set the defines
    std::string mip0resText = std::string("MIP0_RES ") + std::to_string(m_TexData[0].texSize.x);
    m_ShaderDefines.push_back(mip0resText.c_str());
    m_ShaderDefines.push_back("NUM_MIPS 4");

    const CPUMLP& cpuMLP = m_MLPArray[0];
    m_ShaderDefines.push_back(std::string("MLP0_IN_DIM ") + std::to_string(cpuMLP.mlp0Height));
    m_ShaderDefines.push_back(std::string("MLP0_OUT_DIM ") + std::to_string(cpuMLP.mlp0Width));
    m_ShaderDefines.push_back(std::string("MLP1_OUT_DIM ") + std::to_string(cpuMLP.mlp1Width));
    m_ShaderDefines.push_back(std::string("MLP2_OUT_DIM ") + std::to_string(cpuMLP.mlp2Width));

    // Offset buffer
    m_UVOffsetBuffer = graphics::resources::create_graphics_buffer(m_Device, m_UVOffset.size() * sizeof(float2), sizeof(float2), GraphicsBufferType::Default);
    m_TextureSize = { m_TexData[0].texSize.x, m_TexData[0].texSize.y, cpuMLP.finalChannelCount };
}

void TSNC::upload_network(CommandQueue cmdQ, CommandBuffer cmdB)
{
    GraphicsBuffer offsetBufferUp = graphics::resources::create_graphics_buffer(m_Device, m_UVOffset.size() * sizeof(float2), sizeof(float2), GraphicsBufferType::Upload);
    graphics::resources::set_buffer_data(offsetBufferUp, (const char*)m_UVOffset.data(), m_UVOffset.size() * sizeof(float2));
    
    // Upload all the data
    {
        graphics::command_buffer::reset(cmdB);

        // Copy the offsets
        graphics::command_buffer::copy_graphics_buffer(cmdB, offsetBufferUp, m_UVOffsetBuffer);

        // Copy all the mips
        for (uint32_t setIdx = 0; setIdx < m_NumSets; ++setIdx)
        {
            graphics::command_buffer::copy_buffer_into_texture_mips(cmdB, m_TexData[4 * setIdx + 0].texBuffer, 0, (m_TexData[4 * setIdx + 0].texSize.x / 4) * (m_TexData[4 * setIdx + 0].texSize.y / 4) * 8, m_Nwk.tex0, setIdx);
            graphics::command_buffer::copy_buffer_into_texture_mips(cmdB, m_TexData[4 * setIdx + 1].texBuffer, 0, (m_TexData[4 * setIdx + 1].texSize.x / 4) * (m_TexData[4 * setIdx + 1].texSize.y / 4) * 8, m_Nwk.tex1, setIdx);
            graphics::command_buffer::copy_buffer_into_texture_mips(cmdB, m_TexData[4 * setIdx + 2].texBuffer, 0, (m_TexData[4 * setIdx + 2].texSize.x / 4) * (m_TexData[4 * setIdx + 2].texSize.y / 4) * 8, m_Nwk.tex2, setIdx);
            graphics::command_buffer::copy_buffer_into_texture_mips(cmdB, m_TexData[4 * setIdx + 3].texBuffer, 0, (m_TexData[4 * setIdx + 3].texSize.x / 4) * (m_TexData[4 * setIdx + 3].texSize.y / 4) * 8, m_Nwk.tex3, setIdx);
        }

        graphics::command_buffer::close(cmdB);
        graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
        graphics::command_queue::flush(cmdQ);
    }

    // Release the temporary buffers
    graphics::resources::destroy_graphics_buffer(offsetBufferUp);
    for (uint32_t setIdx = 0; setIdx < m_NumSets; ++setIdx)
    {
        graphics::resources::destroy_graphics_buffer(m_TexData[4 * setIdx + 0].texBuffer);
        graphics::resources::destroy_graphics_buffer(m_TexData[4 * setIdx + 1].texBuffer);
        graphics::resources::destroy_graphics_buffer(m_TexData[4 * setIdx + 2].texBuffer);
        graphics::resources::destroy_graphics_buffer(m_TexData[4 * setIdx + 3].texBuffer);
        m_TexData[4 * setIdx + 0].texBuffer = 0;
        m_TexData[4 * setIdx + 1].texBuffer = 0;
        m_TexData[4 * setIdx + 2].texBuffer = 0;
        m_TexData[4 * setIdx + 3].texBuffer = 0;
    }

    {
        // For each buffer, let's concat all the mlps*
        std::vector<float> mlpWeight0, mlpWeight1, mlpWeight2;
        std::vector<float> mlpBias0, mlpBias1, mlpBias2;
        for (uint32_t setIdx = 0; setIdx < m_NumSets; ++setIdx)
        {
            const CPUMLP& cpuMLP = m_MLPArray[setIdx];
            if (m_CVS)
            {
                mlp::upload_and_convert_matrices(m_Device, cmdQ, cmdB, (char*)cpuMLP.mlp0Buffer.data(), cpuMLP.mlp0Width, cpuMLP.mlp0Height, m_Nwk.mlp.weight0Buffer, m_Nwk.mlp.weight0OptimalBuffer, setIdx * cpuMLP.mlp0Width * cpuMLP.mlp0Height * sizeof(float16_t));
                mlp::upload_and_convert_matrices(m_Device, cmdQ, cmdB, (char*)cpuMLP.mlp1Buffer.data(), cpuMLP.mlp1Width, cpuMLP.mlp1Height, m_Nwk.mlp.weight1Buffer, m_Nwk.mlp.weight1OptimalBuffer, setIdx * cpuMLP.mlp0Width * cpuMLP.mlp0Height * sizeof(float16_t));
                mlp::upload_and_convert_matrices(m_Device, cmdQ, cmdB, (char*)cpuMLP.mlp2Buffer.data(), cpuMLP.mlp2Width, cpuMLP.mlp2Height, m_Nwk.mlp.weight2Buffer, m_Nwk.mlp.weight2OptimalBuffer, setIdx * cpuMLP.mlp0Width * cpuMLP.mlp0Height * sizeof(float16_t));
            }
            else
            {
                // Concatenate the bias buffers
                mlpWeight0.insert(mlpWeight0.end(), cpuMLP.mlp0Buffer.begin(), cpuMLP.mlp0Buffer.begin() + cpuMLP.mlp0Width * cpuMLP.mlp0Height);
                mlpWeight1.insert(mlpWeight1.end(), cpuMLP.mlp1Buffer.begin(), cpuMLP.mlp1Buffer.begin() + cpuMLP.mlp1Width * cpuMLP.mlp1Height);
                mlpWeight2.insert(mlpWeight2.end(), cpuMLP.mlp2Buffer.begin(), cpuMLP.mlp2Buffer.begin() + cpuMLP.mlp2Width * cpuMLP.mlp2Height);
            }

            // Concatenate the bias buffers
            mlpBias0.insert(mlpBias0.end(), cpuMLP.mlp0Buffer.begin() + cpuMLP.mlp0Width * cpuMLP.mlp0Height, cpuMLP.mlp0Buffer.end());
            mlpBias1.insert(mlpBias1.end(), cpuMLP.mlp1Buffer.begin() + cpuMLP.mlp1Width * cpuMLP.mlp1Height, cpuMLP.mlp1Buffer.end());
            mlpBias2.insert(mlpBias2.end(), cpuMLP.mlp2Buffer.begin() + cpuMLP.mlp2Width * cpuMLP.mlp2Height, cpuMLP.mlp2Buffer.end());
        }

        // Weight buffers
        if (!m_CVS)
        {
            sync_convert_and_upload_buffer_to_gpu(m_Device, cmdQ, cmdB, m_FP32toFP16CS, (char*)mlpWeight0.data(), mlpWeight0.size() * sizeof(float), sizeof(float), m_Nwk.mlp.weight0Buffer);
            sync_convert_and_upload_buffer_to_gpu(m_Device, cmdQ, cmdB, m_FP32toFP16CS, (char*)mlpWeight1.data(), mlpWeight1.size() * sizeof(float), sizeof(float), m_Nwk.mlp.weight1Buffer);
            sync_convert_and_upload_buffer_to_gpu(m_Device, cmdQ, cmdB, m_FP32toFP16CS, (char*)mlpWeight2.data(), mlpWeight2.size() * sizeof(float), sizeof(float), m_Nwk.mlp.weight2Buffer);
        }

        // Bias buffers
        sync_convert_and_upload_buffer_to_gpu(m_Device, cmdQ, cmdB, m_FP32toFP16CS, (char*)mlpBias0.data(), mlpBias0.size() * sizeof(float), sizeof(float), m_Nwk.mlp.bias0Buffer);
        sync_convert_and_upload_buffer_to_gpu(m_Device, cmdQ, cmdB, m_FP32toFP16CS, (char*)mlpBias1.data(), mlpBias1.size() * sizeof(float), sizeof(float), m_Nwk.mlp.bias1Buffer);
        sync_convert_and_upload_buffer_to_gpu(m_Device, cmdQ, cmdB, m_FP32toFP16CS, (char*)mlpBias2.data(), mlpBias2.size() * sizeof(float), sizeof(float), m_Nwk.mlp.bias2Buffer);
    }
}

void TSNC::reload_shaders(const std::string& shaderLibrary)
{
    ComputeShaderDescriptor csd;
    csd.includeDirectories.push_back(shaderLibrary);
    csd.filename = shaderLibrary + "\\FP32toFP16.compute";
    compile_and_replace_compute_shader(m_Device, csd, m_FP32toFP16CS);
}