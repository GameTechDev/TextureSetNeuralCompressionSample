/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "render_pipeline/tile_classifier.h"
#include "tools/shader_utils.h"

#define WORK_GROUP_SIZE 32

TileClassifier::TileClassifier()
{
}

TileClassifier::~TileClassifier()
{
}

void TileClassifier::initialize(GraphicsDevice device, const uint2& tileSize, uint32_t numMLPS)
{
    // Keep track of the device
    m_Device = device;

    // Keep the size
    m_TileSize = tileSize;
    const uint32_t numTiles = tileSize.x * tileSize.y;

    // Allocate the buffers
    m_ActiveTileBuffer = graphics::resources::create_graphics_buffer(m_Device, (1 + numTiles) * sizeof(uint32_t), sizeof(uint32_t), GraphicsBufferType::Default);
    m_UniformTileBuffer = graphics::resources::create_graphics_buffer(m_Device, (1 + numTiles) * sizeof(uint32_t), sizeof(uint32_t), GraphicsBufferType::Default);
    m_ComplexTileBuffer = graphics::resources::create_graphics_buffer(m_Device, (1 + numTiles) * sizeof(uint32_t), sizeof(uint32_t), GraphicsBufferType::Default);
    m_MLPUsageBuffer = graphics::resources::create_graphics_buffer(m_Device, 2 * numMLPS * sizeof(uint32_t), sizeof(uint32_t), GraphicsBufferType::Default);
    m_RepackedTilesBuffer = graphics::resources::create_graphics_buffer(m_Device, WORK_GROUP_SIZE * numTiles * sizeof(uint32_t), sizeof(uint32_t), GraphicsBufferType::Default);
    m_IndirectBuffer = graphics::resources::create_graphics_buffer(m_Device, 3 * 4 * sizeof(uint32_t), sizeof(uint32_t), GraphicsBufferType::Default, (uint32_t)GraphicsBufferFlags::Indirect);
}

void TileClassifier::release()
{
    // Graphics resources
    graphics::resources::destroy_graphics_buffer(m_ActiveTileBuffer);
    graphics::resources::destroy_graphics_buffer(m_UniformTileBuffer);
    graphics::resources::destroy_graphics_buffer(m_ComplexTileBuffer);
    graphics::resources::destroy_graphics_buffer(m_MLPUsageBuffer);
    graphics::resources::destroy_graphics_buffer(m_RepackedTilesBuffer);
    graphics::resources::destroy_graphics_buffer(m_IndirectBuffer);

    // Shaders
    graphics::compute_shader::destroy_compute_shader(m_PrepareIndirectionCS);
    graphics::compute_shader::destroy_compute_shader(m_ResetCS);
    graphics::compute_shader::destroy_compute_shader(m_FirstPassCS);
    graphics::compute_shader::destroy_compute_shader(m_SecondPassCS);
}

void TileClassifier::reload_shaders(const std::string& shaderLibrary)
{
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Classification\\PrepareIndirection.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_PrepareIndirectionCS);
    }

    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Classification\\Reset.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_ResetCS);
    }

    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Classification\\FirstPass.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_FirstPassCS);
    }

    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Classification\\SecondPass.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_SecondPassCS);
    }
}

void TileClassifier::classify(CommandBuffer cmdB, ConstantBuffer globalCB, RenderTexture visibilityBuffer, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer)
{
    graphics::command_buffer::start_section(cmdB, "Tile classification");

    // Clear the classification data
    {
        // CBVs
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_ResetCS, "_GlobalCB", globalCB);

        // Buffers
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_ResetCS, "_ActiveTileBufferRW", m_ActiveTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_ResetCS, "_UniformTileBufferRW", m_UniformTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_ResetCS, "_ComplexTileBufferRW", m_ComplexTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_ResetCS, "_MLPUsageBufferRW", m_MLPUsageBuffer);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch(cmdB, m_ResetCS, 1, 1, 1);
        graphics::command_buffer::uav_barrier_buffer(cmdB, m_UniformTileBuffer);
    }

    // First classification
    {
        // CBVs
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_FirstPassCS, "_GlobalCB", globalCB);

        // SRVs
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_FirstPassCS, "_VisibilityBuffer", visibilityBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_FirstPassCS, "_VertexBuffer", vertexBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_FirstPassCS, "_IndexBuffer", indexBuffer);

        // UAVs
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_FirstPassCS, "_ActiveTileBufferRW", m_ActiveTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_FirstPassCS, "_UniformTileBufferRW", m_UniformTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_FirstPassCS, "_ComplexTileBufferRW", m_ComplexTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_FirstPassCS, "_MLPUsageBufferRW", m_MLPUsageBuffer);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch(cmdB, m_FirstPassCS, m_TileSize.x, m_TileSize.y, 1);
        graphics::command_buffer::uav_barrier_buffer(cmdB, m_UniformTileBuffer);
    }

    // Prepare the indirection
    {
        // CBVs
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_PrepareIndirectionCS, "_GlobalCB", globalCB);

        // SRVs
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_PrepareIndirectionCS, "_ActiveTileBuffer", m_ActiveTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_PrepareIndirectionCS, "_UniformTileBuffer", m_UniformTileBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_PrepareIndirectionCS, "_ComplexTileBuffer", m_ComplexTileBuffer);

        // UAVs
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_PrepareIndirectionCS, "_MLPUsageBufferRW", m_MLPUsageBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_PrepareIndirectionCS, "_IndirectDispatchBufferRW", m_IndirectBuffer);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch(cmdB, m_PrepareIndirectionCS, 1, 1, 1);
        graphics::command_buffer::uav_barrier_buffer(cmdB, m_IndirectBuffer);
    }

    // Second classification
    {
        // CBUffers
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_SecondPassCS, "_GlobalCB", globalCB);

        // SRVs
        graphics::command_buffer::set_compute_shader_render_texture(cmdB, m_SecondPassCS, "_VisibilityBuffer", visibilityBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SecondPassCS, "_VertexBuffer", vertexBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SecondPassCS, "_IndexBuffer", indexBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SecondPassCS, "_ComplexTileBuffer", m_ComplexTileBuffer);

        // UAVs
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SecondPassCS, "_MLPUsageBufferRW", m_MLPUsageBuffer);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SecondPassCS, "_IndexedTilesBufferRW", m_RepackedTilesBuffer);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch_indirect(cmdB, m_SecondPassCS, m_IndirectBuffer, 6 * sizeof(uint32_t));
    }

    graphics::command_buffer::end_section(cmdB);
}
