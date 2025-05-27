/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Project includes
#include "graphics/types.h"

// System includes
#include <string>

struct TextureSet
{
	Texture tex0 = 0;
	Texture tex1 = 0;
	Texture tex2 = 0;
	Texture tex3 = 0;
	Texture tex4 = 0;
};

class TextureManager
{
public:
	// Cst & Dst
	TextureManager();
	~TextureManager();

	// Init & release
	void initialize(GraphicsDevice device);
	void release();

	// Upload the textures
	void upload_textures(CommandQueue cmdQ, CommandBuffer cmdB, const std::string& modelDir, const std::string& modelName);

	// Returns the texture set
	const TextureSet& texture_set(bool compressed) const {return compressed ? m_BC6Set : m_UncompressedSet;}

private:
	// Graphics device
	GraphicsDevice m_Device = 0;

	// Texture data
	TextureSet m_UncompressedSet = TextureSet();
	TextureSet m_BC6Set = TextureSet();
};