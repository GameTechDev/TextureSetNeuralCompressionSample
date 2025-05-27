/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "tools/stream.h"

// External includes
#include <string>

void pack_buffer(std::vector<char>& buffer, size_t write_size, const char* data)
{
	if (write_size) {
		size_t old_size = buffer.size();
		buffer.resize(old_size + write_size);
		memcpy(buffer.data() + old_size, data, write_size);
	}
}

void unpack_buffer(const char*& stream, size_t read_size, char* data)
{
	if (read_size) {
		memcpy(data, stream, read_size);
		stream += read_size;
	}
}

template<>
void pack_type(std::vector<char>& buffer, const std::string& str)
{
	// Previous size
	const size_t old_size = buffer.size();

	// Num chars
	const uint32_t numChars = (uint32_t)str.size();

	// Allocate the required memory
	buffer.resize(old_size + numChars + sizeof(uint32_t));

	// Copy the size
	memcpy(buffer.data() + old_size, &numChars, sizeof(uint32_t));

	// Copy the char
	if (numChars > 0)
		memcpy(buffer.data() + old_size + sizeof(uint32_t), str.data(), numChars);
}

template<>
void unpack_type(const char*& stream, std::string& str)
{
	// Read the size
	uint32_t numChars;
	memcpy(&numChars, stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	if (numChars > 0)
	{
		// Allocate memory space
		str.resize(numChars);
		memcpy((void*)str.data(), stream, numChars);
		stream += numChars;
	}
}