/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include <graphics/types.h>

#include <render_pipeline/ibl.h>
#include <render_pipeline/types.h>
#include <render_pipeline/texture_manager.h>
#include <render_pipeline/tile_classifier.h>

#include <network/tsnc.h>

// System includes
#include <string>

class MaterialRenderer
{
public:
	// Cst & Dst
	MaterialRenderer();
	~MaterialRenderer();

	// Init and releases
	void initialize(GraphicsDevice device, bool coopVectors);
	void release();

	// Reload shaders
	void reload_shaders(const std::string& shaderLibrary, const TSNC& network);

	// Evaluate the material
	void evaluate_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, 
		GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl, const TextureSet& texSet, FilteringMode filteringMode,
		RenderTexture visilityBuffer, GraphicsBuffer shadowTexture, GraphicsBuffer indexationBuffer, GraphicsBuffer indirectBuffer, RenderTexture colorTexture);

	void evaluate_neural_cmp_indirect(CommandBuffer cmdB, ConstantBuffer globalCB, 
		const TSNC& network, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl, bool useCooperativeVectors, FilteringMode filteringMode,
		RenderTexture visilityBuffer, GraphicsBuffer shadowTexture, const TileClassifier& classifier, RenderTexture colorTexture);

private:
	void partial_inference(CommandBuffer cmdB, ComputeShader targetCS, uint32_t indirectOffset, GraphicsBuffer tileBuffer, ConstantBuffer globalCB,
		const TSNC& network, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, const IBL& ibl, bool useCooperativeVectors, FilteringMode filteringMode,
		RenderTexture visilityBuffer, GraphicsBuffer shadowTexture, const TileClassifier& classifier, RenderTexture colorTexture);

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
	ComputeShader m_TexturesCS = 0;

	// BC1 Inference
	ComputeShader m_FMABC1CS = 0;
	ComputeShader m_CVBC1CS = 0;
	ComputeShader m_FMABC1_Repacked_CS = 0;
	ComputeShader m_CVBC1_Repacked_CS = 0;
};