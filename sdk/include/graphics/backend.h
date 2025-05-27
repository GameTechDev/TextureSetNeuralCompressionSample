/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// SDK includes
#include "graphics/descriptors.h"
#include "graphics/event_collector.h"

namespace graphics
{
	// Setup the current graphics API
	bool setup_graphics_api(GraphicsAPI graphicsAPI);

    namespace device
    {
        // Pre-creation functions
        void enable_experimental_features();
        void enable_debug_layer();

        // Create and destroy
        GraphicsDevice create_graphics_device(DevicePickStrategy pickStrategy = DevicePickStrategy::VRAMSize, uint32_t id = 0);
        void destroy_graphics_device(GraphicsDevice graphicsDevice);

        // Get the additional device info
        GPUVendor get_gpu_vendor(GraphicsDevice device);
        const char* get_device_name(GraphicsDevice device);

        // Feature support
        bool feature_support(GraphicsDevice device, GPUFeature feature);
        CoopMatTier coop_mat_tier(GraphicsDevice device);

        // Stable power state
        void set_stable_power_state(GraphicsDevice device, bool state);
    }

    namespace command_queue
    {
        // Creation and destruction
        CommandQueue create_command_queue(GraphicsDevice graphicsDevice, CommandQueuePriority directPriority = CommandQueuePriority::High,
            CommandQueuePriority computePriority = CommandQueuePriority::Normal,
            CommandQueuePriority copyPriority = CommandQueuePriority::Normal);
        void destroy_command_queue(CommandQueue commandQueue);

        // Operations
        void execute_command_buffer(CommandQueue commandQueue, CommandBuffer commandBuffer, bool swapChain = true);
        void signal(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type = CommandBufferType::Default);
        void wait(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type = CommandBufferType::Default);
        void flush(CommandQueue commandQueue, CommandBufferType type = CommandBufferType::Default);
    }

    // Command Buffer API
    namespace command_buffer
    {
        // Creation and Destruction
        CommandBuffer create_command_buffer(GraphicsDevice graphicsDevice, CommandBufferType commandBufferType = CommandBufferType::Default);
        void destroy_command_buffer(CommandBuffer command_buffer);

        // Generic operations
        void reset(CommandBuffer commandBuffer);
        void close(CommandBuffer commandBuffer);

#pragma region Render Texture
        void clear_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture, const float4& color);
        void clear_depth_texture(CommandBuffer commandBuffer, RenderTexture depthTexture, float value);
        void clear_depth_stencil_texture(CommandBuffer commandBuffer, RenderTexture depthTexture, float depth, uint8_t stencil);
        void clear_stencil_texture(CommandBuffer commandBuffer, RenderTexture stencilTexutre, uint8_t stencil);
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture);
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture, RenderTexture depthTexture);
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture0, RenderTexture renderTexture1, RenderTexture depthTexture);
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture0, RenderTexture renderTexture1, RenderTexture renderTexture2, RenderTexture depthTexture);
#pragma endregion

#pragma region Copy
        void copy_graphics_buffer(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, GraphicsBuffer outputBuffer);
        void copy_graphics_buffer(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint32_t inputOffset, GraphicsBuffer outputBuffer, uint32_t outputOffset, uint64_t size);

        void upload_constant_buffer(CommandBuffer commandBuffer, ConstantBuffer inputBuffer, ConstantBuffer outputBuffer);
        void upload_constant_buffer(CommandBuffer commandBuffer, ConstantBuffer constantBuffer);

        void copy_texture(CommandBuffer commandBuffer, Texture inputTexture, Texture outputTexture);
        void copy_texture(CommandBuffer commandBuffer, RenderTexture inputTexture, uint32_t inputIdx, RenderTexture outputTexture, uint32_t outputIdx);
        void copy_render_texture(CommandBuffer commandBuffer, RenderTexture inputTexture, RenderTexture outputTexture);

        void copy_buffer_into_texture(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, Texture outputTexture, uint32_t sliceIdx, uint32_t mipIdx);
        void copy_buffer_into_texture_mip(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, Texture outputTexture, uint32_t mipIdx);
        void copy_buffer_into_texture_mips(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, uint32_t imageSize, Texture outputTexture, uint32_t sliceIdx);
        void copy_buffer_into_render_texture(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, Texture outputRenderTexture, uint32_t sliceIdx);
        void copy_texture_into_buffer(CommandBuffer commandBuffer, Texture inputTexture, uint32_t sliceIdx, uint32_t mipIdx, GraphicsBuffer outputBuffer, uint64_t bufferOffset);
        void copy_render_texture_into_buffer(CommandBuffer commandBuffer, RenderTexture inputTexture, uint32_t sliceIdx, GraphicsBuffer outputBuffer, uint64_t bufferOffset);
#pragma endregion

#pragma region UAV barrier
        void uav_barrier_buffer(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer);
        void uav_barrier_texture(CommandBuffer commandBuffer, Texture texture);
        void uav_barrier_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture);
#pragma endregion

#pragma region Transitions
        void transition_to_common(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer);
        void transition_to_copy_source(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer);
        void transition_to_present(CommandBuffer commandBuffer, RenderTexture renderTexture);
#pragma endregion

#pragma region Compute Shader
        // Bindings
        void set_compute_shader_cbuffer(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, ConstantBuffer constantBuffer);
        void set_compute_shader_buffer(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, GraphicsBuffer graphicsBuffer);
        void set_compute_shader_texture(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, Texture texture, uint32_t mipLevel = 0);
        void set_compute_shader_render_texture(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, RenderTexture texture);
        void set_compute_shader_sampler(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, Sampler sampler);
        void set_compute_shader_rtas(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, TopLevelAS rtas);

        // Dispatch
        void dispatch(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ);
        void dispatch_indirect(CommandBuffer commandBuffer, ComputeShader computeShader, GraphicsBuffer indirectBuffer, uint32_t offset = 0);
#pragma endregion

#pragma region Graphics Pipeline
        // Viewport
        void set_viewport(CommandBuffer commandBuffer, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height);

        // Bindings
        void set_graphics_pipeline_cbuffer(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, ConstantBuffer constantBuffer);
        void set_graphics_pipeline_buffer(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, GraphicsBuffer graphicsBuffer, uint64_t bufferOffset = 0);
        void set_graphics_pipeline_texture(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, Texture texture);
        void set_graphics_pipeline_render_texture(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, RenderTexture renderTexture);
        void set_graphics_pipeline_sampler(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, Sampler sampler);
        void set_graphics_pipeline_rtas(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, TopLevelAS rtas);

        // Draw
        void draw_indexed(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, uint32_t numTriangles, uint32_t numInstances, DrawPrimitive primitive = DrawPrimitive::Triangle);
        void draw_procedural(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t numTriangles, uint32_t numInstances, DrawPrimitive primitive = DrawPrimitive::Triangle);
        void draw_procedural_indirect(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, GraphicsBuffer indirectBuffer, uint64_t buffeOffset = 0);
#pragma endregion

#pragma region Ray Tracing
        void build_blas(CommandBuffer cmdB, BottomLevelAS blas);
        void build_tlas(CommandBuffer cmdB, TopLevelAS tlas);
#pragma endregion

#pragma region Events
        void start_section(CommandBuffer commandBuffer, const std::string& eventName);
        void end_section(CommandBuffer commandBuffer);
#pragma endregion

#pragma region Ray Tracing
        void enable_profiling_scope(CommandBuffer commandBuffer, ProfilingScope scope);
        void disable_profiling_scope(CommandBuffer commandBuffer, ProfilingScope scope);
#pragma endregion

#pragma region Misc
        void convert_mat_32_to_16(CommandBuffer commandBuffer, GraphicsBuffer inputMatrixBuffer, uint64_t inputOffset, GraphicsBuffer outputMatrixBuffer, uint64_t outputOffset, uint32_t width, uint32_t height, bool optimal);
#pragma endregion
    }

    namespace window
    {
        // Creation and destruction
        RenderWindow create_window(GraphicsDevice device, uint64_t hInstance, uint32_t width, uint32_t height, const char* windowName = "bacasable");
        void destroy_window(RenderWindow renderWindow);

        // Viewport
        void viewport_size(RenderWindow window, uint2& size);
        uint2 viewport_center(RenderWindow window);
        void viewport_bounds(RenderWindow renderWindow, uint4& bounds);

        // Window
        void window_size(RenderWindow window, uint2& size);
        uint2 window_center(RenderWindow window);
        void window_bounds(RenderWindow renderWindow, uint4& bounds);

        // Inputs
        void handle_messages(RenderWindow renderWindow);

        // Manipulation
        void show(RenderWindow renderWindow);
        void hide(RenderWindow renderWindow);

        // Cursor
        void set_cursor_visibility(RenderWindow renderWindow, bool state);
        void set_cursor_pos(RenderWindow renderWindow, uint2 position);
    }

    namespace swap_chain
    {
        // Creation and Destruction
        SwapChain create_swap_chain(RenderWindow window, GraphicsDevice graphicsDevice, CommandQueue commandQueue, TextureFormat format);
        void destroy_swap_chain(SwapChain swapChain);

        // Operations
        RenderTexture get_current_render_texture(SwapChain swapChain);
        void present(SwapChain swapChain, CommandQueue cmQ);
    }

    namespace resources
    {
#pragma region Sampler
        Sampler create_sampler(GraphicsDevice graphicsDevice, const SamplerDescriptor& smplDesc);
        void destroy_sampler(Sampler sampler);
#pragma endregion

#pragma region Texture
        Texture create_texture(GraphicsDevice graphicsDevice, TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipCount, bool isUAV, TextureFormat format, float4 clearColor, const char* debugName);
        Texture create_texture(GraphicsDevice graphicsDevice, const TextureDescriptor& rtDesc);
        void destroy_texture(Texture texture);
        void texture_dimensions(Texture texture, uint32_t& width, uint32_t& height, uint32_t& depth);
#pragma endregion

#pragma region Render Texture
        RenderTexture create_render_texture(GraphicsDevice graphicsDevice, TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipCount, bool isUAV, TextureFormat format, float4 clearColor, const char* debugName);
        RenderTexture create_render_texture(GraphicsDevice graphicsDevice, const TextureDescriptor& rtDesc);
        void destroy_render_texture(RenderTexture renderTexture);
        void render_texture_dimensions(RenderTexture renderTexture, uint32_t& width, uint32_t& height, uint32_t& depth);
#pragma endregion

#pragma region Graphics Buffer
        GraphicsBuffer create_graphics_buffer(GraphicsDevice graphicsDevice, uint64_t bufferSize, uint32_t elementSize, GraphicsBufferType bufferType = GraphicsBufferType::Default, uint32_t bufferFlags = 0);
        void destroy_graphics_buffer(GraphicsBuffer graphicsBuffer);
        void set_buffer_data(GraphicsBuffer graphicsBuffer, const char* buffer, uint64_t bufferSize, uint32_t bufferOffset = 0);
        char* allocate_cpu_buffer(GraphicsBuffer graphicsBuffer);
        void release_cpu_buffer(GraphicsBuffer graphicsBuffer);
        void set_buffer_debug_name(GraphicsBuffer graphicsBuffer, const char* name);
#pragma endregion

#pragma region Constant Buffer
        ConstantBuffer create_constant_buffer(GraphicsDevice graphicsDevice, uint32_t elementSize, ConstantBufferType bufferType);
        void destroy_constant_buffer(ConstantBuffer constantBuffer);
        void set_constant_buffer(ConstantBuffer constantBuffer, const char* bufferData, uint32_t bufferSize);
#pragma endregion

#pragma region BLAS
        BottomLevelAS create_blas(GraphicsDevice device, GraphicsBuffer vertexBuffer, uint32_t vertexCount, GraphicsBuffer indexBuffer, uint32_t numTriangles, uint32_t positionStride = sizeof(float3));
        void destroy_blas(BottomLevelAS blas);
#pragma endregion

#pragma region TLAS
        TopLevelAS create_tlas(GraphicsDevice device, uint32_t numBLAS);
        void destroy_tlas(TopLevelAS tlas);
        void set_tlas_instance(TopLevelAS tlas, BottomLevelAS blas, uint32_t index);
        void upload_tlas_instance_data(TopLevelAS tlas);
#pragma endregion
    }

    namespace compute_shader
    {
        ComputeShader create_compute_shader(GraphicsDevice graphicsDevice, const ComputeShaderDescriptor& computeShaderDescriptor, bool experimental = false);
        void destroy_compute_shader(ComputeShader computeShader);
    }

    namespace graphics_pipeline
    {
        GraphicsPipeline create_graphics_pipeline(GraphicsDevice graphicsDevice, const GraphicsPipelineDescriptor& graphicsPipelineDescriptor);
        void destroy_graphics_pipeline(GraphicsPipeline graphicsPipeline);
        void set_stencil_ref(GraphicsPipeline graphicsPipeline, uint8_t stencilRef);
    }

    namespace profiling_scope
    {
        ProfilingScope create_profiling_scope(GraphicsDevice graphicsDevice);
        void destroy_profiling_scope(ProfilingScope profilingScope);
        uint64_t get_duration_us(ProfilingScope profilingScope, CommandQueue cmdQ, CommandBufferType type = CommandBufferType::Default);
    }

    namespace imgui
    {
        // Init & Dst
        bool initialize_imgui(GraphicsDevice device, RenderWindow window, TextureFormat format);
        void release_imgui();

        // Runtime functions
        void start_frame();
        void end_frame();
        void draw_frame(CommandBuffer cmd, RenderTexture renderTexture);
        void handle_input(RenderWindow window, const EventData& data);
    }
}