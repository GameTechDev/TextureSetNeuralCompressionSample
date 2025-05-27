/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "network/mlp.h"
#include "graphics/backend.h"
#include "tools/stream.h"
#include "tools/gpu_helpers.h"

namespace mlp
{
    void allocate_gpu_mlp(GraphicsDevice device, const CPUMLP& cpuMLP, GPUMLP& gpuMLP)
    {
        // Layer 0
        gpuMLP.weight0Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp0Width * cpuMLP.mlp0Height * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.weight0OptimalBuffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp0Width * cpuMLP.mlp0Height * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.bias0Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp0Width * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);

        // Layer 1
        gpuMLP.weight1Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp1Width * cpuMLP.mlp1Height * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.weight1OptimalBuffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp1Width * cpuMLP.mlp1Height * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.bias1Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp1Width * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);

        // Layer 2
        gpuMLP.weight2Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp2Width * cpuMLP.mlp2Height * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.weight2OptimalBuffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp2Width * cpuMLP.mlp2Height * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.bias2Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp2Width * sizeof(float16_t), sizeof(float16_t), GraphicsBufferType::Default);
    }

    void allocate_gpu_mlp_array(GraphicsDevice device, const std::vector<CPUMLP>& cpuMLPArray, GPUMLP& gpuMLP)
    {
        const CPUMLP& cpuMLP = cpuMLPArray[0];
        uint64_t numMLPs = cpuMLPArray.size();

        // Layer 0
        gpuMLP.weight0Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp0Width * cpuMLP.mlp0Height * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.weight0OptimalBuffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp0Width * cpuMLP.mlp0Height * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.bias0Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp0Width * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);

        // Layer 1
        gpuMLP.weight1Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp1Width * cpuMLP.mlp1Height * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.weight1OptimalBuffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp1Width * cpuMLP.mlp1Height * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.bias1Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp1Width * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);

        // Layer 2
        gpuMLP.weight2Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp2Width * cpuMLP.mlp2Height * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.weight2OptimalBuffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp2Width * cpuMLP.mlp2Height * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);
        gpuMLP.bias2Buffer = graphics::resources::create_graphics_buffer(device, cpuMLP.mlp2Width * sizeof(float16_t) * numMLPs, sizeof(float16_t), GraphicsBufferType::Default);
    }

    void align_dimensions(CPUMLP& mlp)
    {
        // Align the paletK input on 16
        uint32_t v = mlp.mlp0Height % 16;
        if (v != 0)
        {
            // The actual size we want
            uint32_t newHeight = ((mlp.mlp0Height + 15) / 16) * 16;
            std::vector<float> data(mlp.mlp0Width * newHeight + mlp.mlp0Width);

            // Copy the weights
            memcpy((char*)&data[0], (char*)&mlp.mlp0Buffer[0], sizeof(float) * mlp.mlp0Width * mlp.mlp0Height);

            // Copy the bias
            memcpy((char*)&data[mlp.mlp0Width * newHeight], (char*)&mlp.mlp0Buffer[mlp.mlp0Width * mlp.mlp0Height], sizeof(float) * mlp.mlp0Width);

            // Assign
            mlp.mlp0Height = newHeight;
            mlp.mlp0Buffer = data;
        }

        // Align the output on 16
        v = mlp.mlp2Width % 16;
        if (v != 0)
        {
            // The actual size we want
            uint32_t targetWidth = ((mlp.mlp2Width + 15) / 16) * 16;

            // Create the new data
            std::vector<float> data(mlp.mlp2Height * targetWidth + targetWidth);
            for (uint32_t y = 0; y < mlp.mlp2Height; ++y)
                memcpy((char*)&data[targetWidth * y], (char*)&mlp.mlp2Buffer[mlp.mlp2Width * y], sizeof(float) * mlp.mlp2Width);
            memcpy((char*)&data[targetWidth * mlp.mlp2Height], (char*)&mlp.mlp2Buffer[mlp.mlp2Width * mlp.mlp2Height], sizeof(float) * mlp.mlp2Width);

            // Update the sizes
            mlp.mlp2Width = targetWidth;
            mlp.mlp2Buffer = data;
            mlp.finalChannelCount = targetWidth;
        }
    }

    void upload_and_convert_matrices(GraphicsDevice device, CommandQueue queue, CommandBuffer cmdB, const char* buffer, uint32_t matrixWidth, uint32_t matrixHeight, GraphicsBuffer mainBuffer, GraphicsBuffer optimalBuffer, uint64_t offsetBuffer)
    {
        // Input buffer size
        const uint32_t bufferSize = matrixWidth * matrixHeight * sizeof(float);

        // Create the graphics buffer to upload, process and readback the bitfield buffer
        GraphicsBuffer uploadBuffer = graphics::resources::create_graphics_buffer(device, bufferSize, sizeof(float), GraphicsBufferType::Upload);
        GraphicsBuffer tmpBuffer = graphics::resources::create_graphics_buffer(device, bufferSize, sizeof(float), GraphicsBufferType::Default);

        // Upload the initial bitfield
        graphics::resources::set_buffer_data(uploadBuffer, buffer, bufferSize);

        // Reset the command buffer
        graphics::command_buffer::reset(cmdB);

        graphics::command_buffer::copy_graphics_buffer(cmdB, uploadBuffer, tmpBuffer);
        if (optimalBuffer != 0)
            graphics::command_buffer::convert_mat_32_to_16(cmdB, tmpBuffer, 0, optimalBuffer, offsetBuffer, matrixWidth, matrixHeight, true);
        if (mainBuffer != 0)
            graphics::command_buffer::convert_mat_32_to_16(cmdB, tmpBuffer, 0, mainBuffer, offsetBuffer, matrixWidth, matrixHeight, false);

        // Close the command buffer
        graphics::command_buffer::close(cmdB);

        // Execute the command buffer in the command queue
        graphics::command_queue::execute_command_buffer(queue, cmdB);

        // Flush the queue
        graphics::command_queue::flush(queue);

        // Make sure to free the graphics buffer
        graphics::resources::destroy_graphics_buffer(uploadBuffer);
        graphics::resources::destroy_graphics_buffer(tmpBuffer);
    }

    void upload(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, ComputeShader fp32tofp16CS, const CPUMLP& cpuMLP, GPUMLP& gpuMLP)
    {
        // MLP0
        upload_and_convert_matrices(device, cmdQ, cmdB, (char*)cpuMLP.mlp0Buffer.data(), cpuMLP.mlp0Width, cpuMLP.mlp0Height, gpuMLP.weight0Buffer, gpuMLP.weight0OptimalBuffer, 0);
        sync_convert_and_upload_buffer_to_gpu(device, cmdQ, cmdB, fp32tofp16CS, (char*)(cpuMLP.mlp0Buffer.data() + cpuMLP.mlp0Width * cpuMLP.mlp0Height), cpuMLP.mlp0Width * sizeof(float), sizeof(float), gpuMLP.bias0Buffer);

        // MLP1
        upload_and_convert_matrices(device, cmdQ, cmdB, (char*)cpuMLP.mlp1Buffer.data(), cpuMLP.mlp1Width, cpuMLP.mlp1Height, gpuMLP.weight1Buffer, gpuMLP.weight1OptimalBuffer, 0);
        sync_convert_and_upload_buffer_to_gpu(device, cmdQ, cmdB, fp32tofp16CS, (char*)(cpuMLP.mlp1Buffer.data() + cpuMLP.mlp1Width * cpuMLP.mlp1Height), cpuMLP.mlp1Width * sizeof(float), sizeof(float), gpuMLP.bias1Buffer);

        // MLP2
        upload_and_convert_matrices(device, cmdQ, cmdB, (char*)cpuMLP.mlp2Buffer.data(), cpuMLP.mlp2Width, cpuMLP.mlp2Height, gpuMLP.weight2Buffer, gpuMLP.weight2OptimalBuffer, 0);
        sync_convert_and_upload_buffer_to_gpu(device, cmdQ, cmdB, fp32tofp16CS, (char*)(cpuMLP.mlp2Buffer.data() + cpuMLP.mlp2Width * cpuMLP.mlp2Height), cpuMLP.mlp2Width * sizeof(float), sizeof(float), gpuMLP.bias2Buffer);
    }

    // Free the allocated memory
    void destroy_gpu_mlp(GPUMLP& gpuMLP)
    {
        graphics::resources::destroy_graphics_buffer(gpuMLP.weight0Buffer);
        graphics::resources::destroy_graphics_buffer(gpuMLP.bias0Buffer);

        graphics::resources::destroy_graphics_buffer(gpuMLP.weight1Buffer);
        graphics::resources::destroy_graphics_buffer(gpuMLP.bias1Buffer);

        graphics::resources::destroy_graphics_buffer(gpuMLP.weight2Buffer);
        graphics::resources::destroy_graphics_buffer(gpuMLP.bias2Buffer);

        if (gpuMLP.weight0OptimalBuffer)
            graphics::resources::destroy_graphics_buffer(gpuMLP.weight0OptimalBuffer);
        if (gpuMLP.weight1OptimalBuffer)
            graphics::resources::destroy_graphics_buffer(gpuMLP.weight1OptimalBuffer);
        if (gpuMLP.weight2OptimalBuffer)
            graphics::resources::destroy_graphics_buffer(gpuMLP.weight2OptimalBuffer);
    }
}

void unpack_type(const char*& stream, CPUMLP& mlp)
{
    // MLP data
    unpack_bytes<uint32_t>(stream, mlp.nbMlp);
    unpack_bytes<uint32_t>(stream, mlp.finalChannelCount);
    unpack_bytes<uint32_t>(stream, mlp.finalBlockWidth);

    // MLP layer 0
    unpack_bytes<uint32_t>(stream, mlp.mlp0Width);
    unpack_bytes<uint32_t>(stream, mlp.mlp0Height);
    const uint32_t mlp0Size = mlp.mlp0Width * mlp.mlp0Height + mlp.mlp0Width;
    mlp.mlp0Buffer.resize(mlp0Size);
    unpack_buffer(stream, mlp0Size * sizeof(float), (char*)mlp.mlp0Buffer.data());

    // MLP layer 1
    unpack_bytes<uint32_t>(stream, mlp.mlp1Width);
    unpack_bytes<uint32_t>(stream, mlp.mlp1Height);
    const uint32_t mlp1Size = mlp.mlp1Width * mlp.mlp1Height + mlp.mlp1Width;
    mlp.mlp1Buffer.resize(mlp1Size);
    unpack_buffer(stream, mlp1Size * sizeof(float), (char*)mlp.mlp1Buffer.data());

    // MLP layer 2
    unpack_bytes<uint32_t>(stream, mlp.mlp2Width);
    unpack_bytes<uint32_t>(stream, mlp.mlp2Height);
    const uint32_t mlp2Size = mlp.mlp2Width * mlp.mlp2Height + mlp.mlp2Width;
    mlp.mlp2Buffer.resize(mlp2Size);
    unpack_buffer(stream, mlp2Size * sizeof(float), (char*)mlp.mlp2Buffer.data());
}
