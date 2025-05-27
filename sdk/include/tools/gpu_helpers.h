/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "graphics/types.h"

// Helper function to upload data to the GPU
// WARNING: These function are a terrible implementation, they allocate memory, do a synchronous upload and free the GPU memory, but they are convinent so they are used.
void sync_convert_and_upload_buffer_to_gpu(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, ComputeShader convertCS, const char* cpuBuffer, uint64_t bufferSize, uint32_t elementSize, GraphicsBuffer convertedBuffer, GraphicsBuffer rawBuffer = 0);
void sync_upload_buffer_to_gpu(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const char* cpuBuffer, uint64_t bufferSize, uint32_t elementSize, GraphicsBuffer targetBuffer);
