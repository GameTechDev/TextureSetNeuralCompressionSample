/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "tools/security.h"
#include "tools/texture_utils.h"
#include "tools/stream.h"

// System includes
#include <iostream>
#include <fstream>
#include <vector>

GraphicsBuffer load_bc1_to_graphics_buffer(GraphicsDevice device, const char* texturePath, uint3& dimensions, float2& uvOffset)
{
	// Vector that will hold our packed mesh 
	std::vector<char> binaryFile;

	// Read from disk
	FILE* pFile;
	pFile = fopen(texturePath, "rb");
	fseek(pFile, 0L, SEEK_END);
	size_t fileSize = _ftelli64(pFile);
	binaryFile.resize(fileSize);
	_fseeki64(pFile, 0L, SEEK_SET);
	rewind(pFile);
	fread(binaryFile.data(), sizeof(char), fileSize, pFile);
	fclose(pFile);

	// Read the sizes
	uint32_t* intArray = (uint32_t*)binaryFile.data();
	float* floatArray = (float*)binaryFile.data();
	dimensions.x = intArray[0] * 4;
	dimensions.y = intArray[1] * 4;
	dimensions.z = intArray[2];
	uvOffset.x = floatArray[3];
	uvOffset.y = floatArray[4];

	uint32_t sizesOffset = sizeof(uint32_t) * 5;
	dimensions.z = std::max(1, (int32_t)dimensions.z - 2);
	const uint32_t bufferSize = (uint32_t)binaryFile.size() - sizesOffset;

	// Create the buffer, upload to it and return it
	GraphicsBuffer textureBuffer = graphics::resources::create_graphics_buffer(device, bufferSize, 4, GraphicsBufferType::Upload);
	graphics::resources::set_buffer_data(textureBuffer, binaryFile.data() + sizesOffset, bufferSize);
	return textureBuffer;
}

GraphicsBuffer load_bc6_to_graphics_buffer(GraphicsDevice device, const char* texturePath, uint32_t& width, uint32_t& height, uint32_t& mipCount)
{
	// Vector that will hold our packed mesh 
	std::vector<char> binaryFile;

	// Read from disk
	FILE* pFile;
	pFile = fopen(texturePath, "rb");
	fseek(pFile, 0L, SEEK_END);
	size_t fileSize = _ftelli64(pFile);
	binaryFile.resize(fileSize);
	_fseeki64(pFile, 0L, SEEK_SET);
	rewind(pFile);
	fread(binaryFile.data(), sizeof(char), fileSize, pFile);
	fclose(pFile);

	// Read the sizes
	uint32_t* intArray = (uint32_t*)binaryFile.data();
	width = intArray[0] * 4;
	height = intArray[1] * 4;
	mipCount = intArray[2];
	uint32_t sizesOffset = sizeof(uint32_t) * 3;
	mipCount = std::max(1, (int32_t)mipCount - 2);
	const uint32_t bufferSize = (uint32_t)binaryFile.size() - sizesOffset;

	// Create the buffer, upload to it and return it
	GraphicsBuffer textureBuffer = graphics::resources::create_graphics_buffer(device, bufferSize, 4, GraphicsBufferType::Upload);
	graphics::resources::set_buffer_data(textureBuffer, binaryFile.data() + sizesOffset, bufferSize);
	return textureBuffer;
}

namespace binary_texture
{
	void import_binary_texture(const char* path, BinaryTexture& bt)
	{
		// Vector that will hold our packed mesh 
		std::vector<char> binaryFile;

		// Read from disk
		FILE* pFile;
		pFile = fopen(path, "rb");
		fseek(pFile, 0L, SEEK_END);
		size_t fileSize = _ftelli64(pFile);
		binaryFile.resize(fileSize);
		_fseeki64(pFile, 0L, SEEK_SET);
		rewind(pFile);
		fread(binaryFile.data(), sizeof(char), fileSize, pFile);
		fclose(pFile);

		// Pack the structure in a buffer
		const char* binaryPtr = binaryFile.data();
		unpack_bytes(binaryPtr, bt.width);
		unpack_bytes(binaryPtr, bt.height);
		unpack_bytes(binaryPtr, bt.depth);
		unpack_bytes(binaryPtr, bt.mipCount);
		unpack_bytes(binaryPtr, bt.format);
		unpack_bytes(binaryPtr, bt.type);
		unpack_vector_bytes(binaryPtr, bt.data);
	}

	void export_binary_texture(const BinaryTexture& bt, const char* path)
	{
		// Vector that will hold our packed mesh 
		std::vector<char> binaryFile;

		// Per-tetra data
		pack_bytes(binaryFile, bt.width);
		pack_bytes(binaryFile, bt.height);
		pack_bytes(binaryFile, bt.depth);
		pack_bytes(binaryFile, bt.mipCount);
		pack_bytes(binaryFile, bt.format);
		pack_bytes(binaryFile, bt.type);
		pack_vector_bytes(binaryFile, bt.data);

		// Write to disk
		FILE* pFile;
		pFile = fopen(path, "wb");
		fwrite(binaryFile.data(), sizeof(char), binaryFile.size(), pFile);
		fclose(pFile);
	}
}