/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Project includes
#include <graphics/types.h>

#include <render_pipeline/ibl.h>
#include <render_pipeline/types.h>
#include <render_pipeline/texture_manager.h>
#include <render_pipeline/tile_classifier.h>

#include <network/tsnc.h>

// System includes
#include <string>

class GBufferRenderer
{
public:
	// Cst & Dst
	GBufferRenderer();
	~GBufferRenderer();

	// Init and releases
	void initialize(GraphicsDevice device, bool coopVectors);
	void release();

	// Reload network
	void reload_shaders(const std::string& shaderLibrary, const std::vector<std::string>& shaderDefines);

	// Evaluate the network
	void evaluate_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, GraphicsBuffer visibilityBuffer, GraphicsBuffer indexationBuffer, GraphicsBuffer indirectBuffer, GraphicsBuffer outputBuffer,
		const TextureSet& texSet, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, FilteringMode filteringMode);

	// Evaluate the network
	void evaluate_neural_cmp_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, GraphicsBuffer visibilityBuffer, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, GraphicsBuffer outputBuffer,
		const TileClassifier& classifier, bool useCoopVectors, const TSNC& network, FilteringMode filteringMode);

	// Lighting pass
	void lighting_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl,
		GraphicsBuffer gbuffer, GraphicsBuffer tileBuffer, GraphicsBuffer indirectBuffer, 
		RenderTexture visibilityBuffer, RenderTexture shadowTexture, RenderTexture colorTexture);

private:
	void partial_inference(CommandBuffer cmdB, ComputeShader repackedCS, uint32_t indirectOffset, GraphicsBuffer tileBuffer, ConstantBuffer globalCB, GraphicsBuffer visibilityBuffer, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, GraphicsBuffer outputBuffer,
		const TileClassifier& classifier, bool useCoopVectors, const TSNC& network, FilteringMode filteringMode);

private:
	// Graphics device
	GraphicsDevice m_Device = 0;

	// State tracking
	bool m_CoopVectors = false;

	// Samplers
	Sampler m_NearestSampler = 0;
	Sampler m_LinearSampler = 0;
	Sampler m_AnisoSampler = 0;

	// Texture inference
	ComputeShader m_TextureCS = 0;

	// BC1 inference
	ComputeShader m_FMABC1CS = 0;
	ComputeShader m_CVBC1CS = 0;

	ComputeShader m_FMABC1_Repacked_CS = 0;
	ComputeShader m_CVBC1_Repacked_CS = 0;

	// Lighting shader
	ComputeShader m_DeferredLightingCS = 0;
};