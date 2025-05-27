/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "graphics/types.h"

// System includes
#include <string>

class TileClassifier
{
public:
	// Cst & Dst
	TileClassifier();
	~TileClassifier();

	// Init & release
	void initialize(GraphicsDevice device, const uint2& tileSize, uint32_t numMLPS);
	void release();

	// Resource loading
	void reload_shaders(const std::string& shaderLibrary);

	// Runtime
	void classify(CommandBuffer cmdB, ConstantBuffer globalCB, RenderTexture visibilityBuffer, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer);

	// Resource access
	GraphicsBuffer active_tiles_buffer() const { return m_ActiveTileBuffer; }
	GraphicsBuffer uniform_tiles_buffer() const { return m_UniformTileBuffer; }
	GraphicsBuffer complex_tiles_buffer() const { return m_ComplexTileBuffer; }
	GraphicsBuffer repacked_tiles_buffer() const { return m_RepackedTilesBuffer; }
	GraphicsBuffer indirect_buffer() const { return m_IndirectBuffer; }

private:
	// Device
	GraphicsDevice m_Device = 0;

	// Shaders
	ComputeShader m_PrepareIndirectionCS = 0;
	ComputeShader m_ResetCS = 0;
	ComputeShader m_FirstPassCS = 0;
	ComputeShader m_SecondPassCS = 0;

	// Runtime resources
	GraphicsBuffer m_ActiveTileBuffer = 0;
	GraphicsBuffer m_UniformTileBuffer = 0;
	GraphicsBuffer m_ComplexTileBuffer = 0;
	GraphicsBuffer m_MLPUsageBuffer = 0;
	GraphicsBuffer m_RepackedTilesBuffer = 0;
	GraphicsBuffer m_IndirectBuffer = 0;

	// Other data
	uint2 m_TileSize = { 0, 0 };
};