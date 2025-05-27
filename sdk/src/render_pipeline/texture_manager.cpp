/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "render_pipeline/texture_manager.h"
#include "tools/directory_utilities.h"
#include "tools/texture_utils.h"

Texture read_binary_texture_and_upload(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const std::string& texFile)
{
	// Read from disk
	BinaryTexture binTex;
	binary_texture::import_binary_texture(texFile.c_str(), binTex);

	// Allocate the texture
	TextureDescriptor desc;
	desc.type = binTex.type;
	desc.width = binTex.width;
	desc.height = binTex.height;
	desc.depth = binTex.depth;
	desc.mipCount = binTex.mipCount;
	desc.format = binTex.format;
	Texture tex = graphics::resources::create_texture(device, desc);

	// Create the upload buffer
	GraphicsBuffer imageBuffer = graphics::resources::create_graphics_buffer(device, binTex.data.size(), sizeof(uint32_t), GraphicsBufferType::Upload);
	graphics::resources::set_buffer_data(imageBuffer, (const char*)binTex.data.data(), binTex.data.size());

	// Copy the buffer to a texture
	graphics::command_buffer::reset(cmdB);
	graphics::command_buffer::copy_buffer_into_texture_mips(cmdB, imageBuffer, 0, sizeof(uint32_t) * binTex.width * binTex.height, tex, 0);
	graphics::command_buffer::close(cmdB);
	graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
	graphics::command_queue::flush(cmdQ);

	// Destroy the graphics buffer
	graphics::resources::destroy_graphics_buffer(imageBuffer);

	// return the texture
	return tex;
}

Texture read_bc6_texture_and_upload(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const std::string& texFile)
{
	std::vector<char> binaryData;
	load_file_to_array(texFile.c_str(), binaryData);

	// Create the upload buffer
	uint32_t width, height, mipCount;
	GraphicsBuffer imageBuffer = load_bc6_to_graphics_buffer(device, texFile.c_str(), width, height, mipCount);

	// Allocate the texture
	TextureDescriptor desc;
	desc.type = TextureType::Tex2D;
	desc.width = width;
	desc.height = height;
	desc.depth = 1;
	desc.mipCount = mipCount;
	desc.format = TextureFormat::BC6_RGB;
	Texture tex = graphics::resources::create_texture(device, desc);

	// Copy the buffer to a texture
	graphics::command_buffer::reset(cmdB);
	graphics::command_buffer::copy_buffer_into_texture_mips(cmdB, imageBuffer, 0, (width / 4) * (height / 4) * 16, tex, 0);
	graphics::command_buffer::close(cmdB);
	graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
	graphics::command_queue::flush(cmdQ);

	// Destroy the graphics buffer
	graphics::resources::destroy_graphics_buffer(imageBuffer);

	// return the texture
	return tex;
}

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
}

void TextureManager::initialize(GraphicsDevice device)
{
	// Keep track of the device
	m_Device = device;
}

void TextureManager::release()
{
	// Uncompressed
	graphics::resources::destroy_texture(m_UncompressedSet.tex0);
	graphics::resources::destroy_texture(m_UncompressedSet.tex1);
	graphics::resources::destroy_texture(m_UncompressedSet.tex2);
	graphics::resources::destroy_texture(m_UncompressedSet.tex3);
	graphics::resources::destroy_texture(m_UncompressedSet.tex4);

	// BC6 set
	graphics::resources::destroy_texture(m_BC6Set.tex0);
	graphics::resources::destroy_texture(m_BC6Set.tex1);
	graphics::resources::destroy_texture(m_BC6Set.tex2);
	graphics::resources::destroy_texture(m_BC6Set.tex3);
	graphics::resources::destroy_texture(m_BC6Set.tex4);
}

void TextureManager::upload_textures(CommandQueue cmdQ, CommandBuffer cmdB, const std::string& modelDir, const std::string& modelName)
{
	// Uncompressed textures
	{
		const std::string tex0Path = modelDir + "\\" + modelName + "\\uncompressed\\tex0.tex_bin";
		m_UncompressedSet.tex0 = read_binary_texture_and_upload(m_Device, cmdQ, cmdB, tex0Path);

		const std::string tex1Path = modelDir + "\\" + modelName + "\\uncompressed\\tex1.tex_bin";
		m_UncompressedSet.tex1 = read_binary_texture_and_upload(m_Device, cmdQ, cmdB, tex1Path);

		const std::string tex2Path = modelDir + "\\" + modelName + "\\uncompressed\\tex2.tex_bin";
		m_UncompressedSet.tex2 = read_binary_texture_and_upload(m_Device, cmdQ, cmdB, tex2Path);

		const std::string tex3Path = modelDir + "\\" + modelName + "\\uncompressed\\tex3.tex_bin";
		m_UncompressedSet.tex3 = read_binary_texture_and_upload(m_Device, cmdQ, cmdB, tex3Path);

		const std::string tex4Path = modelDir + "\\" + modelName + "\\uncompressed\\tex4.tex_bin";
		m_UncompressedSet.tex4 = read_binary_texture_and_upload(m_Device, cmdQ, cmdB, tex4Path);
	}

	// Uncompressed textures
	{
		const std::string tex0Path = modelDir + "\\" + modelName + "\\bc6\\tex0.bc6";
		m_BC6Set.tex0 = read_bc6_texture_and_upload(m_Device, cmdQ, cmdB, tex0Path);

		const std::string tex1Path = modelDir + "\\" + modelName + "\\bc6\\tex1.bc6";
		m_BC6Set.tex1 = read_bc6_texture_and_upload(m_Device, cmdQ, cmdB, tex1Path);

		const std::string tex2Path = modelDir + "\\" + modelName + "\\bc6\\tex2.bc6";
		m_BC6Set.tex2 = read_bc6_texture_and_upload(m_Device, cmdQ, cmdB, tex2Path);

		const std::string tex3Path = modelDir + "\\" + modelName + "\\bc6\\tex3.bc6";
		m_BC6Set.tex3 = read_bc6_texture_and_upload(m_Device, cmdQ, cmdB, tex3Path);

		const std::string tex4Path = modelDir + "\\" + modelName + "\\bc6\\tex4.bc6";
		m_BC6Set.tex4 = read_bc6_texture_and_upload(m_Device, cmdQ, cmdB, tex4Path);
	}
}