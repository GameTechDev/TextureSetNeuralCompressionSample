/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Project includes
#include "network/mlp.h"

// System includes
#include <string>
#include <vector>

// Latent space texture data
struct LSTextureData
{
	// Texture size (width, height, mipcount)
	uint3 texSize = { 0, 0, 0 };
	// Raw buffer that contains the latent space as a BC1 texture
	GraphicsBuffer texBuffer = 0;
};

// GPU representation of the compressed network
struct GPUNetworkCompressed
{
	// Latent space textures
	Texture tex0 = 0;
	Texture tex1 = 0;
	Texture tex2 = 0;
	Texture tex3 = 0;

	// MLP
	GPUMLP mlp;
};

class TSNC
{
public:
	// Cst & Dst
	TSNC();
	~TSNC();

	// Init and releases
	void initialize(GraphicsDevice device, bool cvs);
	void release();

	// Reload resources
	void reload_network(const std::string& modelDir, uint32_t numSets);
	void reload_shaders(const std::string& shaderLibrary);
	void upload_network(CommandQueue cmdQ, CommandBuffer cmdB);

	// Network data access
	const GPUNetworkCompressed& gpu_network() const { return m_Nwk; }
	const GraphicsBuffer& uv_offset_buffer() const { return m_UVOffsetBuffer; }
	const std::vector<std::string>& shader_defines() const { return m_ShaderDefines; }
	uint3 texture_size() const { return m_TextureSize; }

protected:
	// Device
	GraphicsDevice m_Device = 0;
	bool m_CVS = false;

	// Number of sets
	uint32_t m_NumSets = 0;
	// Sampled resolution
	uint3 m_TextureSize = { 0, 0, 0 };
	// Latent space texture data (compressed)
	std::vector<LSTextureData> m_TexData;
	// MLP data (CPU)
	std::vector<CPUMLP> m_MLPArray;
	// UV offsets used 
	std::vector<float2> m_UVOffset;
	std::vector<std::string> m_ShaderDefines;

	// GPU data
	GraphicsBuffer m_UVOffsetBuffer = 0;
	GPUNetworkCompressed m_Nwk = GPUNetworkCompressed();

	// Shaders
	ComputeShader m_FP32toFP16CS = 0;
};
