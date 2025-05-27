/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// System includes
#include <vector>

// Functions to pack/unpack a raw buffer (knwoing its size in both cases
void pack_buffer(std::vector<char>& buffer, size_t write_size, const char* data);
void unpack_buffer(const char*& stream, size_t read_size, char* data);

// Functions to pack/unpack a type T as bytes
template<typename T>
void pack_bytes(std::vector<char>& buffer, const T& type);
template<typename T>
void unpack_bytes(const char*& stream, T& type);

// Functions to pack/unpack a vector and its elements as types
template<typename T>
void pack_vector_types(std::vector<char>& buffer, const std::vector<T>& data);
template<typename T>
void unpack_vector_types(const char*& stream, std::vector<T>& data);

// Function to pack/unpack a buffer and its content as bytes
template<typename T>
void pack_vector_bytes(std::vector<char>& buffer, const std::vector<T>& data);
template<typename T>
void unpack_vector_bytes(const char*& stream, std::vector<T>& data);

// Function to pack types
// Functions to pack/unpack a type T as bytes
template<typename T>
void pack_type(std::vector<char>& buffer, const T& type);
template<typename T>
void unpack_type(const char*& stream, T& type);

#include "stream.inl"