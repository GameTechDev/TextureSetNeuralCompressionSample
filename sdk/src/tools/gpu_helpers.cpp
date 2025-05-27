/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "tools/gpu_helpers.h"

#define CONVERT_KERNEL_WORKGROUP_SIZE 1024

void sync_convert_and_upload_buffer_to_gpu(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, ComputeShader convertCS,
    const char* cpuBuffer, uint64_t bufferSize, uint32_t elementSize, GraphicsBuffer convertedBuffer, GraphicsBuffer rawBuffer)
{
    // Element count
    const uint64_t numElements = bufferSize / elementSize;

    // Create the graphics buffer used to upload
    GraphicsBuffer uploadBuffer = graphics::resources::create_graphics_buffer(device, bufferSize, elementSize, GraphicsBufferType::Upload);

    // Upload the to the buffer
    graphics::resources::set_buffer_data(uploadBuffer, cpuBuffer, bufferSize);

    // Reset the command buffer
    graphics::command_buffer::reset(cmdB);

    // Copy the input buffer to the processing buffers
    graphics::command_buffer::set_compute_shader_buffer(cmdB, convertCS, "_InputBuffer", uploadBuffer);
    graphics::command_buffer::set_compute_shader_buffer(cmdB, convertCS, "_OutputBufferRW", convertedBuffer);
    graphics::command_buffer::dispatch(cmdB, convertCS, (uint32_t)((numElements + CONVERT_KERNEL_WORKGROUP_SIZE - 1) / CONVERT_KERNEL_WORKGROUP_SIZE), 1, 1);

    // Only copy the raw one if defined
    if (rawBuffer != 0)
        graphics::command_buffer::copy_graphics_buffer(cmdB, uploadBuffer, rawBuffer);

    // Close the command buffer
    graphics::command_buffer::close(cmdB);

    // Execute the command buffer in the command queue
    graphics::command_queue::execute_command_buffer(cmdQ, cmdB);

    // Flush the queue
    graphics::command_queue::flush(cmdQ);

    // Make sure to free the graphics buffer
    graphics::resources::destroy_graphics_buffer(uploadBuffer);
}

void sync_upload_buffer_to_gpu(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const char* cpuBuffer, uint64_t bufferSize, uint32_t elementSize, GraphicsBuffer targetBuffer)
{
    // Create the graphics buffer to upload, process and readback the bitfield buffer
    GraphicsBuffer uploadBuffer = graphics::resources::create_graphics_buffer(device, bufferSize, elementSize, GraphicsBufferType::Upload);

    // Upload the initial bitfield
    graphics::resources::set_buffer_data(uploadBuffer, cpuBuffer, bufferSize);

    // Reset the command buffer
    graphics::command_buffer::reset(cmdB);

    // Copy the input buffer to the processing buffers
    graphics::command_buffer::copy_graphics_buffer(cmdB, uploadBuffer, targetBuffer);

    // Close the command buffer
    graphics::command_buffer::close(cmdB);

    // Execute the command buffer in the command queue
    graphics::command_queue::execute_command_buffer(cmdQ, cmdB);

    // Flush the queue
    graphics::command_queue::flush(cmdQ);

    // Make sure to free the graphics buffer
    graphics::resources::destroy_graphics_buffer(uploadBuffer);
}