/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "math/types.h"
#include "graphics/types.h"

// System includes
#include <vector>

// CPU representation of the MLP
struct CPUMLP
{
	uint32_t nbMlp;
	uint32_t finalChannelCount;
	uint32_t finalBlockWidth;

	// MLP layer 0
	uint32_t mlp0Width;
	uint32_t mlp0Height;
	std::vector<float> mlp0Buffer;

	// Activation: ReLU(x) 

	// MLP layer 1
	uint32_t mlp1Width;
	uint32_t mlp1Height;
	std::vector<float> mlp1Buffer;

	// Activation: ReLU(x) 

	// MLP layer 2
	uint32_t mlp2Width;
	uint32_t mlp2Height;
	std::vector<float> mlp2Buffer;
};

// GPU representation of the MLP
struct GPUMLP
{
	// MLP layer 0
	GraphicsBuffer weight0Buffer = 0;
	GraphicsBuffer weight0OptimalBuffer = 0;
	GraphicsBuffer bias0Buffer = 0;

	// MLP layer 1
	GraphicsBuffer weight1Buffer = 0;
	GraphicsBuffer weight1OptimalBuffer = 0;
	GraphicsBuffer bias1Buffer = 0;

	// MLP layer 2
	GraphicsBuffer weight2Buffer = 0;
	GraphicsBuffer weight2OptimalBuffer = 0;
	GraphicsBuffer bias2Buffer = 0;
};

namespace mlp
{
	// Adjust to fit to multiples of 16
	void align_dimensions(CPUMLP& mlp);

	// Allocate GPU buffers
	void allocate_gpu_mlp(GraphicsDevice device, const CPUMLP& cpuMLP, GPUMLP& gpuMLP);
	void allocate_gpu_mlp_array(GraphicsDevice device, const std::vector<CPUMLP>& cpuMLPArray, GPUMLP& gpuMLP);

	// Free the allocated memory
	void destroy_gpu_mlp(GPUMLP& gpuMLP);

	// Upload the MLP buffers to the GPU
	void upload_and_convert_matrices(GraphicsDevice device, CommandQueue queue, CommandBuffer cmdB, const char* buffer, uint32_t matrixWidth, uint32_t matrixHeight, GraphicsBuffer mainBuffer, GraphicsBuffer optimalBuffer, uint64_t offsetBuffer);
	void upload(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, ComputeShader fp32tofp16CS, const CPUMLP& cpuMLP, GPUMLP& gpuMLP);
}

// Unpacks the CPU MLP from a stream
void unpack_type(const char*& stream, CPUMLP& mlp);
