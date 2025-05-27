/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#if defined(D3D12_SUPPORTED)
#include "dx12/dx12_backend.h"
#endif
#include "tools/security.h"

struct BackendPointers
{
#pragma region device
    void (*__device__enable_experimental_features)() = nullptr;
    void (*__device__enable_debug_layer)() = nullptr;
    GraphicsDevice(*__device__create_graphics_device)(DevicePickStrategy pickStrategy, uint32_t id) = nullptr;
    void (*__device__destroy_graphics_device)(GraphicsDevice graphicsDevice) = nullptr;
    GPUVendor(*__device__get_gpu_vendor)(GraphicsDevice graphicsDevice) = nullptr;
    const char* (*__device__get_device_name)(GraphicsDevice graphicsDevice) = nullptr;
    bool (*__device__feature_support)(GraphicsDevice device, GPUFeature feature) = nullptr;
    CoopMatTier (*__device__coop_mat_tier)(GraphicsDevice device) = nullptr;
    void(*__device__set_stable_power_state)(GraphicsDevice device, bool state) = nullptr;
#pragma endregion

#pragma region command_queue
    CommandQueue (*__command_queue__create_command_queue)(GraphicsDevice graphicsDevice, CommandQueuePriority directPriority, CommandQueuePriority computePriority, CommandQueuePriority copyPriority) = nullptr;
    void (*__command_queue__destroy_command_queue)(CommandQueue commandQueue) = nullptr;
    void (*__command_queue__execute_command_buffer)(CommandQueue commandQueue, CommandBuffer commandBuffer, bool swapChain) = nullptr;
    void (*__command_queue__signal)(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type) = nullptr;
    void (*__command_queue__wait)(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type) = nullptr;
    void (*__command_queue__flush)(CommandQueue commandQueue, CommandBufferType type) = nullptr;
#pragma endregion

#pragma region command_buffer
    // Creation and Destruction
    CommandBuffer(*__command_buffer__create_command_buffer)(GraphicsDevice graphicsDevice, CommandBufferType commandBufferType) = nullptr;
    void (*__command_buffer__destroy_command_buffer)(CommandBuffer command_buffer) = nullptr;

    // Generic operations
    void (*__command_buffer__reset)(CommandBuffer commandBuffer) = nullptr;
    void (*__command_buffer__close)(CommandBuffer commandBuffer) = nullptr;

    // Render Texture
    void (*__command_buffer__clear_render_texture)(CommandBuffer, RenderTexture, const float4&) = nullptr;
    void (*__command_buffer__clear_depth_texture)(CommandBuffer, RenderTexture, float) = nullptr;
    void (*__command_buffer__clear_depth_stencil_texture)(CommandBuffer, RenderTexture, float, uint8_t) = nullptr;
    void (*__command_buffer__clear_stencil_texture)(CommandBuffer, RenderTexture, uint8_t) = nullptr;
    void (*__command_buffer__set_render_texture_1)(CommandBuffer, RenderTexture) = nullptr;
    void (*__command_buffer__set_render_texture_2)(CommandBuffer, RenderTexture, RenderTexture) = nullptr;
    void (*__command_buffer__set_render_texture_3)(CommandBuffer, RenderTexture, RenderTexture, RenderTexture) = nullptr;
    void (*__command_buffer__set_render_texture_4)(CommandBuffer, RenderTexture, RenderTexture, RenderTexture, RenderTexture) = nullptr;

    // Copy
    void (*__command_buffer__copy_graphics_buffer_1)(CommandBuffer, GraphicsBuffer, GraphicsBuffer) = nullptr;
    void (*__command_buffer__copy_graphics_buffer_2)(CommandBuffer, GraphicsBuffer, uint32_t, GraphicsBuffer, uint32_t, uint64_t) = nullptr;
    void (*__command_buffer__upload_constant_buffer_1)(CommandBuffer, ConstantBuffer, ConstantBuffer) = nullptr;
    void (*__command_buffer__upload_constant_buffer_2)(CommandBuffer, ConstantBuffer) = nullptr;
    void (*__command_buffer__copy_texture_1)(CommandBuffer, Texture, Texture) = nullptr;
    void (*__command_buffer__copy_texture_2)(CommandBuffer, RenderTexture, uint32_t, RenderTexture, uint32_t) = nullptr;
    void (*__command_buffer__copy_render_texture)(CommandBuffer, RenderTexture, RenderTexture) = nullptr;
    void (*__command_buffer__copy_buffer_into_texture)(CommandBuffer, GraphicsBuffer, uint64_t, Texture, uint32_t, uint32_t) = nullptr;
    void (*__command_buffer__copy_buffer_into_texture_mip)(CommandBuffer, GraphicsBuffer, uint64_t, Texture, uint32_t) = nullptr;
    void (*__command_buffer__copy_buffer_into_texture_mips)(CommandBuffer, GraphicsBuffer, uint64_t, uint32_t, Texture, uint32_t) = nullptr;
    void (*__command_buffer__copy_buffer_into_render_texture)(CommandBuffer, GraphicsBuffer, uint64_t, Texture, uint32_t) = nullptr;
    void (*__command_buffer__copy_texture_into_buffer)(CommandBuffer, Texture, uint32_t, uint32_t, GraphicsBuffer, uint64_t) = nullptr;
    void (*__command_buffer__copy_render_texture_into_buffer)(CommandBuffer, RenderTexture, uint32_t, GraphicsBuffer, uint64_t) = nullptr;

    // UAV Barrier
    void (*__command_buffer__uav_barrier_buffer)(CommandBuffer, GraphicsBuffer) = nullptr;
    void (*__command_buffer__uav_barrier_texture)(CommandBuffer, Texture) = nullptr;
    void (*__command_buffer__uav_barrier_render_texture)(CommandBuffer, RenderTexture) = nullptr;

    // Transitions
    void (*__command_buffer__transition_to_common)(CommandBuffer, GraphicsBuffer) = nullptr;
    void (*__command_buffer__transition_to_copy_source)(CommandBuffer, GraphicsBuffer) = nullptr;
    void (*__command_buffer__transition_to_present)(CommandBuffer, RenderTexture) = nullptr;

    // Compute Shader
    void (*__command_buffer__set_compute_shader_cbuffer)(CommandBuffer, ComputeShader, const char*, ConstantBuffer) = nullptr;
    void (*__command_buffer__set_compute_shader_buffer)(CommandBuffer, ComputeShader, const char*, GraphicsBuffer) = nullptr;
    void (*__command_buffer__set_compute_shader_texture)(CommandBuffer, ComputeShader, const char*, Texture, uint32_t) = nullptr;
    void (*__command_buffer__set_compute_shader_render_texture)(CommandBuffer, ComputeShader, const char*, RenderTexture) = nullptr;
    void (*__command_buffer__set_compute_shader_sampler)(CommandBuffer, ComputeShader, const char*, Sampler) = nullptr;
    void (*__command_buffer__set_compute_shader_rtas)(CommandBuffer, ComputeShader, const char*, TopLevelAS) = nullptr;
    void (*__command_buffer__dispatch)(CommandBuffer, ComputeShader, uint32_t, uint32_t, uint32_t) = nullptr;
    void (*__command_buffer__dispatch_indirect)(CommandBuffer, ComputeShader, GraphicsBuffer, uint32_t) = nullptr;

    // Graphics Pipeline
    void (*__command_buffer__set_viewport)(CommandBuffer, int32_t, int32_t, uint32_t, uint32_t) = nullptr;
    void (*__command_buffer__set_graphics_pipeline_cbuffer)(CommandBuffer, GraphicsPipeline, const char*, ConstantBuffer) = nullptr;
    void (*__command_buffer__set_graphics_pipeline_buffer)(CommandBuffer, GraphicsPipeline, const char*, GraphicsBuffer, uint64_t) = nullptr;
    void (*__command_buffer__set_graphics_pipeline_texture)(CommandBuffer, GraphicsPipeline, const char*, Texture) = nullptr;
    void (*__command_buffer__set_graphics_pipeline_render_texture)(CommandBuffer, GraphicsPipeline, const char*, RenderTexture) = nullptr;
    void (*__command_buffer__set_graphics_pipeline_sampler)(CommandBuffer, GraphicsPipeline, const char*, Sampler) = nullptr;
    void (*__command_buffer__set_graphics_pipeline_rtas)(CommandBuffer, GraphicsPipeline, const char*, TopLevelAS) = nullptr;
    void (*__command_buffer__draw_indexed)(CommandBuffer, GraphicsPipeline, GraphicsBuffer, GraphicsBuffer, uint32_t, uint32_t, DrawPrimitive) = nullptr;
    void (*__command_buffer__draw_procedural)(CommandBuffer, GraphicsPipeline, uint32_t, uint32_t, DrawPrimitive) = nullptr;
    void (*__command_buffer__draw_procedural_indirect)(CommandBuffer, GraphicsPipeline, GraphicsBuffer, uint64_t) = nullptr;

    // Ray Tracing
    void (*__command_buffer__build_blas)(CommandBuffer, BottomLevelAS) = nullptr;
    void (*__command_buffer__build_tlas)(CommandBuffer, TopLevelAS) = nullptr;

    // Events
    void (*__command_buffer__start_section)(CommandBuffer, const std::string&) = nullptr;
    void (*__command_buffer__end_section)(CommandBuffer) = nullptr;

    // Profiling
    void (*__command_buffer__enable_profiling_scope)(CommandBuffer, ProfilingScope) = nullptr;
    void (*__command_buffer__disable_profiling_scope)(CommandBuffer, ProfilingScope) = nullptr;

    // Misc
    void (*__command_buffer__convert_mat_32_to_16)(CommandBuffer, GraphicsBuffer, uint64_t, GraphicsBuffer, uint64_t, uint32_t, uint32_t, bool) = nullptr;
#pragma endregion

#pragma region window
    RenderWindow (*__window__create_window)(GraphicsDevice device, uint64_t hInstance, uint32_t width, uint32_t height, const char* windowName) = nullptr;
    void (*__window__destroy_window)(RenderWindow renderWindow) = nullptr;
    void (*__window__viewport_size)(RenderWindow window, uint2& size) = nullptr;
    uint2 (*__window__viewport_center)(RenderWindow window) = nullptr;
    void (*__window__viewport_bounds)(RenderWindow renderWindow, uint4& bounds) = nullptr;
    void (*__window__window_size)(RenderWindow window, uint2& size) = nullptr;
    uint2 (*__window__window_center)(RenderWindow window) = nullptr;
    void (*__window__window_bounds)(RenderWindow renderWindow, uint4& bounds) = nullptr;
    void (*__window__handle_messages)(RenderWindow renderWindow) = nullptr;
    void (*__window__show)(RenderWindow renderWindow) = nullptr;
    void (*__window__hide)(RenderWindow renderWindow) = nullptr;
    void (*__window__set_cursor_visibility)(RenderWindow renderWindow, bool state) = nullptr;
    void (*__window__set_cursor_pos)(RenderWindow renderWindow, uint2 position) = nullptr;
#pragma endregion

#pragma region swap_chain
    SwapChain(*__swap_chain__create_swap_chain)(RenderWindow window, GraphicsDevice graphicsDevice, CommandQueue commandQueue, TextureFormat format) = nullptr;
    void (*__swap_chain__destroy_swap_chain)(SwapChain swapChain) = nullptr;
    RenderTexture(*__swap_chain__get_current_render_texture)(SwapChain swapChain) = nullptr;
    void (*__swap_chain__present)(SwapChain swapChain, CommandQueue cmQ) = nullptr;
#pragma endregion

#pragma region graphics_resources
    Sampler(*__graphics_resources__create_sampler)(GraphicsDevice graphicsDevice, const SamplerDescriptor& smplDesc) = nullptr;
    void (*__graphics_resources__destroy_sampler)(Sampler sampler) = nullptr;

    Texture(*__graphics_resources__create_texture_1)(GraphicsDevice, TextureType, uint32_t, uint32_t, uint32_t, uint32_t, bool, TextureFormat, float4, const char*) = nullptr;
    Texture(*__graphics_resources__create_texture_2)(GraphicsDevice, const TextureDescriptor&) = nullptr;
    void (*__graphics_resources__destroy_texture)(Texture texture) = nullptr;
    void (*__graphics_resources__texture_dimensions)(Texture texture, uint32_t& width, uint32_t& height, uint32_t& depth) = nullptr;

    RenderTexture(*__graphics_resources__create_render_texture_1)(GraphicsDevice, TextureType, uint32_t, uint32_t, uint32_t, uint32_t, bool, TextureFormat, float4, const char*) = nullptr;
    RenderTexture(*__graphics_resources__create_render_texture_2)(GraphicsDevice, const TextureDescriptor&) = nullptr;
    void (*__graphics_resources__destroy_render_texture)(RenderTexture renderTexture) = nullptr;
    void (*__graphics_resources__render_texture_dimensions)(RenderTexture renderTexture, uint32_t& width, uint32_t& height, uint32_t& depth) = nullptr;

    GraphicsBuffer(*__graphics_resources__create_graphics_buffer)(GraphicsDevice, uint64_t, uint32_t, GraphicsBufferType, uint32_t) = nullptr;
    void (*__graphics_resources__destroy_graphics_buffer)(GraphicsBuffer) = nullptr;
    void (*__graphics_resources__set_buffer_data)(GraphicsBuffer, const char*, uint64_t, uint32_t) = nullptr;
    char* (*__graphics_resources__allocate_cpu_buffer)(GraphicsBuffer) = nullptr;
    void (*__graphics_resources__release_cpu_buffer)(GraphicsBuffer) = nullptr;
    void (*__graphics_resources__set_buffer_debug_name)(GraphicsBuffer, const char*) = nullptr;

    ConstantBuffer(*__graphics_resources__create_constant_buffer)(GraphicsDevice, uint32_t, ConstantBufferType) = nullptr;
    void (*__graphics_resources__destroy_constant_buffer)(ConstantBuffer) = nullptr;
    void (*__graphics_resources__set_constant_buffer)(ConstantBuffer, const char*, uint32_t) = nullptr;

    BottomLevelAS(*__graphics_resources__create_blas)(GraphicsDevice, GraphicsBuffer, uint32_t, GraphicsBuffer, uint32_t, uint32_t) = nullptr;
    void (*__graphics_resources__destroy_blas)(BottomLevelAS) = nullptr;

    TopLevelAS(*__graphics_resources__create_tlas)(GraphicsDevice, uint32_t) = nullptr;
    void (*__graphics_resources__destroy_tlas)(TopLevelAS) = nullptr;
    void (*__graphics_resources__set_tlas_instance)(TopLevelAS, BottomLevelAS, uint32_t) = nullptr;
    void (*__graphics_resources__upload_tlas_instance_data)(TopLevelAS) = nullptr;
#pragma endregion

#pragma region compute_shader
    ComputeShader(*__compute_shader__create_compute_shader)(GraphicsDevice graphicsDevice, const ComputeShaderDescriptor& computeShaderDescriptor, bool experimental) = nullptr;
    void (*__compute_shader__destroy_compute_shader)(ComputeShader computeShader) = nullptr;
#pragma endregion

#pragma region graphics_pipeline
    GraphicsPipeline(*__graphics_pipeline__create_graphics_pipeline)(GraphicsDevice graphicsDevice, const GraphicsPipelineDescriptor& graphicsPipelineDescriptor) = nullptr;
    void (*__graphics_pipeline__destroy_graphics_pipeline)(GraphicsPipeline graphicsPipeline) = nullptr;
    void (*__graphics_pipeline__set_stencil_ref)(GraphicsPipeline graphicsPipeline, uint8_t stencilRef) = nullptr;
#pragma endregion

#pragma region profiling_scope
    ProfilingScope (*__profiling_scope__create_profiling_scope) (GraphicsDevice graphicsDevice) = nullptr;
    void (*__profiling_scope__destroy_profiling_scope) (ProfilingScope profilingScope) = nullptr;
    uint64_t (*__profiling_scope__get_duration_us) (ProfilingScope profilingScope, CommandQueue cmdQ, CommandBufferType type) = nullptr;
#pragma endregion

#pragma region imgui
    bool (*__imgui__initialize_imgui)(GraphicsDevice device, RenderWindow window, TextureFormat format) = nullptr;
    void (*__imgui__release_imgui)() = nullptr;

    void (*__imgui__start_frame)() = nullptr;
    void (*__imgui__end_frame)() = nullptr;
    void (*__imgui__draw_frame)(CommandBuffer cmd, RenderTexture renderTexture) = nullptr;
    void (*__imgui__handle_input)(RenderWindow window, const EventData& data) = nullptr;
#pragma endregion
};

// Global pointers
BackendPointers g_Backend;

namespace graphics
{
    bool setup_graphics_api(GraphicsAPI graphicsAPI)
    {
        switch (graphicsAPI)
        {
            case GraphicsAPI::DX12:
    #if defined(D3D12_SUPPORTED)
                // Device
                g_Backend.__device__enable_experimental_features = d3d12::device::enable_experimental_features;
                g_Backend.__device__enable_debug_layer = d3d12::device::enable_debug_layer;
                g_Backend.__device__create_graphics_device = d3d12::device::create_graphics_device;
                g_Backend.__device__destroy_graphics_device = d3d12::device::destroy_graphics_device;
                g_Backend.__device__get_gpu_vendor = d3d12::device::get_gpu_vendor;
                g_Backend.__device__get_device_name = d3d12::device::get_device_name;
                g_Backend.__device__feature_support = d3d12::device::feature_support;
                g_Backend.__device__coop_mat_tier = d3d12::device::coop_mat_tier;
                g_Backend.__device__set_stable_power_state = d3d12::device::set_stable_power_state;

                // Command Queue
                g_Backend.__command_queue__create_command_queue = d3d12::command_queue::create_command_queue;
                g_Backend.__command_queue__destroy_command_queue = d3d12::command_queue::destroy_command_queue;
                g_Backend.__command_queue__execute_command_buffer = d3d12::command_queue::execute_command_buffer;
                g_Backend.__command_queue__signal = d3d12::command_queue::signal;
                g_Backend.__command_queue__wait = d3d12::command_queue::wait;
                g_Backend.__command_queue__flush = d3d12::command_queue::flush;

                // Command Buffer
                g_Backend.__command_buffer__create_command_buffer = d3d12::command_buffer::create_command_buffer;
                g_Backend.__command_buffer__destroy_command_buffer = d3d12::command_buffer::destroy_command_buffer;
                g_Backend.__command_buffer__reset = d3d12::command_buffer::reset;
                g_Backend.__command_buffer__close = d3d12::command_buffer::close;
                g_Backend.__command_buffer__clear_render_texture = d3d12::command_buffer::clear_render_texture;
                g_Backend.__command_buffer__clear_depth_texture = d3d12::command_buffer::clear_depth_texture;
                g_Backend.__command_buffer__clear_depth_stencil_texture = d3d12::command_buffer::clear_depth_stencil_texture;
                g_Backend.__command_buffer__clear_stencil_texture = d3d12::command_buffer::clear_stencil_texture;
                g_Backend.__command_buffer__set_render_texture_1 = d3d12::command_buffer::set_render_texture;
                g_Backend.__command_buffer__set_render_texture_2 = d3d12::command_buffer::set_render_texture;
                g_Backend.__command_buffer__set_render_texture_3 = d3d12::command_buffer::set_render_texture;
                g_Backend.__command_buffer__set_render_texture_4 = d3d12::command_buffer::set_render_texture;
                g_Backend.__command_buffer__copy_graphics_buffer_1 = d3d12::command_buffer::copy_graphics_buffer;
                g_Backend.__command_buffer__copy_graphics_buffer_2 = d3d12::command_buffer::copy_graphics_buffer;
                g_Backend.__command_buffer__upload_constant_buffer_1 = d3d12::command_buffer::upload_constant_buffer;
                g_Backend.__command_buffer__upload_constant_buffer_2 = d3d12::command_buffer::upload_constant_buffer;
                g_Backend.__command_buffer__copy_texture_1 = d3d12::command_buffer::copy_texture;
                g_Backend.__command_buffer__copy_texture_2 = d3d12::command_buffer::copy_texture;
                g_Backend.__command_buffer__copy_render_texture = d3d12::command_buffer::copy_render_texture;
                g_Backend.__command_buffer__copy_buffer_into_texture = d3d12::command_buffer::copy_buffer_into_texture;
                g_Backend.__command_buffer__copy_buffer_into_texture_mip = d3d12::command_buffer::copy_buffer_into_texture_mip;
                g_Backend.__command_buffer__copy_buffer_into_texture_mips = d3d12::command_buffer::copy_buffer_into_texture_mips;
                g_Backend.__command_buffer__copy_buffer_into_render_texture = d3d12::command_buffer::copy_buffer_into_render_texture;
                g_Backend.__command_buffer__copy_texture_into_buffer = d3d12::command_buffer::copy_texture_into_buffer;
                g_Backend.__command_buffer__copy_render_texture_into_buffer = d3d12::command_buffer::copy_render_texture_into_buffer;
                g_Backend.__command_buffer__uav_barrier_buffer = d3d12::command_buffer::uav_barrier_buffer;
                g_Backend.__command_buffer__uav_barrier_texture = d3d12::command_buffer::uav_barrier_texture;
                g_Backend.__command_buffer__uav_barrier_render_texture = d3d12::command_buffer::uav_barrier_render_texture;
                g_Backend.__command_buffer__transition_to_common = d3d12::command_buffer::transition_to_common;
                g_Backend.__command_buffer__transition_to_copy_source = d3d12::command_buffer::transition_to_copy_source;
                g_Backend.__command_buffer__transition_to_present = d3d12::command_buffer::transition_to_present;
                g_Backend.__command_buffer__set_compute_shader_cbuffer = d3d12::command_buffer::set_compute_shader_cbuffer;
                g_Backend.__command_buffer__set_compute_shader_buffer = d3d12::command_buffer::set_compute_shader_buffer;
                g_Backend.__command_buffer__set_compute_shader_texture = d3d12::command_buffer::set_compute_shader_texture;
                g_Backend.__command_buffer__set_compute_shader_render_texture = d3d12::command_buffer::set_compute_shader_render_texture;
                g_Backend.__command_buffer__set_compute_shader_sampler = d3d12::command_buffer::set_compute_shader_sampler;
                g_Backend.__command_buffer__set_compute_shader_rtas = d3d12::command_buffer::set_compute_shader_rtas;
                g_Backend.__command_buffer__dispatch = d3d12::command_buffer::dispatch;
                g_Backend.__command_buffer__dispatch_indirect = d3d12::command_buffer::dispatch_indirect;
                g_Backend.__command_buffer__set_viewport = d3d12::command_buffer::set_viewport;
                g_Backend.__command_buffer__set_graphics_pipeline_cbuffer = d3d12::command_buffer::set_graphics_pipeline_cbuffer;
                g_Backend.__command_buffer__set_graphics_pipeline_buffer = d3d12::command_buffer::set_graphics_pipeline_buffer;
                g_Backend.__command_buffer__set_graphics_pipeline_texture = d3d12::command_buffer::set_graphics_pipeline_texture;
                g_Backend.__command_buffer__set_graphics_pipeline_render_texture = d3d12::command_buffer::set_graphics_pipeline_render_texture;
                g_Backend.__command_buffer__set_graphics_pipeline_sampler = d3d12::command_buffer::set_graphics_pipeline_sampler;
                g_Backend.__command_buffer__set_graphics_pipeline_rtas = d3d12::command_buffer::set_graphics_pipeline_rtas;
                g_Backend.__command_buffer__draw_indexed = d3d12::command_buffer::draw_indexed;
                g_Backend.__command_buffer__draw_procedural = d3d12::command_buffer::draw_procedural;
                g_Backend.__command_buffer__draw_procedural_indirect = d3d12::command_buffer::draw_procedural_indirect;
                g_Backend.__command_buffer__build_blas = d3d12::command_buffer::build_blas;
                g_Backend.__command_buffer__build_tlas = d3d12::command_buffer::build_tlas;
                g_Backend.__command_buffer__start_section = d3d12::command_buffer::start_section;
                g_Backend.__command_buffer__end_section = d3d12::command_buffer::end_section;
                g_Backend.__command_buffer__enable_profiling_scope = d3d12::command_buffer::enable_profiling_scope;
                g_Backend.__command_buffer__disable_profiling_scope = d3d12::command_buffer::disable_profiling_scope;
                g_Backend.__command_buffer__convert_mat_32_to_16 = d3d12::command_buffer::convert_mat_32_to_16;

                // Window
                g_Backend.__window__create_window = d3d12::window::create_window;
                g_Backend.__window__destroy_window = d3d12::window::destroy_window;
                g_Backend.__window__viewport_size = d3d12::window::viewport_size;
                g_Backend.__window__viewport_center = d3d12::window::viewport_center;
                g_Backend.__window__viewport_bounds = d3d12::window::viewport_bounds;
                g_Backend.__window__window_size = d3d12::window::window_size;
                g_Backend.__window__window_center = d3d12::window::window_center;
                g_Backend.__window__window_bounds = d3d12::window::window_bounds;
                g_Backend.__window__handle_messages = d3d12::window::handle_messages;
                g_Backend.__window__show = d3d12::window::show;
                g_Backend.__window__hide = d3d12::window::hide;
                g_Backend.__window__set_cursor_visibility = d3d12::window::set_cursor_visibility;
                g_Backend.__window__set_cursor_pos = d3d12::window::set_cursor_pos;

                // Swap Chain
                g_Backend.__swap_chain__create_swap_chain = d3d12::swap_chain::create_swap_chain;
                g_Backend.__swap_chain__destroy_swap_chain = d3d12::swap_chain::destroy_swap_chain;
                g_Backend.__swap_chain__get_current_render_texture = d3d12::swap_chain::get_current_render_texture;
                g_Backend.__swap_chain__present = d3d12::swap_chain::present;

                // Graphics Resources
                g_Backend.__graphics_resources__create_sampler = d3d12::resources::create_sampler;
                g_Backend.__graphics_resources__destroy_sampler = d3d12::resources::destroy_sampler;
                g_Backend.__graphics_resources__create_texture_1 = d3d12::resources::create_texture;
                g_Backend.__graphics_resources__create_texture_2 = d3d12::resources::create_texture;
                g_Backend.__graphics_resources__destroy_texture = d3d12::resources::destroy_texture;
                g_Backend.__graphics_resources__texture_dimensions = d3d12::resources::texture_dimensions;
                g_Backend.__graphics_resources__create_render_texture_1 = d3d12::resources::create_render_texture;
                g_Backend.__graphics_resources__create_render_texture_2 = d3d12::resources::create_render_texture;
                g_Backend.__graphics_resources__destroy_render_texture = d3d12::resources::destroy_render_texture;
                g_Backend.__graphics_resources__render_texture_dimensions = d3d12::resources::render_texture_dimensions;
                g_Backend.__graphics_resources__create_graphics_buffer = d3d12::resources::create_graphics_buffer;
                g_Backend.__graphics_resources__destroy_graphics_buffer = d3d12::resources::destroy_graphics_buffer;
                g_Backend.__graphics_resources__set_buffer_data = d3d12::resources::set_buffer_data;
                g_Backend.__graphics_resources__allocate_cpu_buffer = d3d12::resources::allocate_cpu_buffer;
                g_Backend.__graphics_resources__release_cpu_buffer = d3d12::resources::release_cpu_buffer;
                g_Backend.__graphics_resources__set_buffer_debug_name = d3d12::resources::set_buffer_debug_name;
                g_Backend.__graphics_resources__create_constant_buffer = d3d12::resources::create_constant_buffer;
                g_Backend.__graphics_resources__destroy_constant_buffer = d3d12::resources::destroy_constant_buffer;
                g_Backend.__graphics_resources__set_constant_buffer = d3d12::resources::set_constant_buffer;
                g_Backend.__graphics_resources__create_blas = d3d12::resources::create_blas;
                g_Backend.__graphics_resources__destroy_blas = d3d12::resources::destroy_blas;
                g_Backend.__graphics_resources__create_tlas = d3d12::resources::create_tlas;
                g_Backend.__graphics_resources__destroy_tlas = d3d12::resources::destroy_tlas;
                g_Backend.__graphics_resources__set_tlas_instance = d3d12::resources::set_tlas_instance;
                g_Backend.__graphics_resources__upload_tlas_instance_data = d3d12::resources::upload_tlas_instance_data;

                // Compute shader
                g_Backend.__compute_shader__create_compute_shader = d3d12::compute_shader::create_compute_shader;
                g_Backend.__compute_shader__destroy_compute_shader = d3d12::compute_shader::destroy_compute_shader;

                // Graphics Pipeline
                g_Backend.__graphics_pipeline__create_graphics_pipeline = d3d12::graphics_pipeline::create_graphics_pipeline;
                g_Backend.__graphics_pipeline__destroy_graphics_pipeline = d3d12::graphics_pipeline::destroy_graphics_pipeline;
                g_Backend.__graphics_pipeline__set_stencil_ref = d3d12::graphics_pipeline::set_stencil_ref;

                // Profiling scope
                g_Backend.__profiling_scope__create_profiling_scope = d3d12::profiling_scope::create_profiling_scope;
                g_Backend.__profiling_scope__destroy_profiling_scope = d3d12::profiling_scope::destroy_profiling_scope;
                g_Backend.__profiling_scope__get_duration_us = d3d12::profiling_scope::get_duration_us;

                // IMGUI
                g_Backend.__imgui__initialize_imgui = d3d12::imgui::initialize_imgui;
                g_Backend.__imgui__release_imgui = d3d12::imgui::release_imgui;
                g_Backend.__imgui__start_frame = d3d12::imgui::start_frame;
                g_Backend.__imgui__end_frame = d3d12::imgui::end_frame;
                g_Backend.__imgui__draw_frame = d3d12::imgui::draw_frame;
                g_Backend.__imgui__handle_input = d3d12::imgui::handle_input;

                // All pointers set, valid state
                printf("DX12 API set up.\n");
                return true;
#else
                printf("DX12 not supported by this build.\n");
#endif
            break;
            default:
                printf("Unknown graphics API.\n");
        }

        return false;
    }

    namespace device
    {
        void enable_experimental_features() { g_Backend.__device__enable_experimental_features();}
        void enable_debug_layer() { g_Backend.__device__enable_debug_layer();}
        GraphicsDevice create_graphics_device(DevicePickStrategy pickStrategy, uint32_t id) { return g_Backend.__device__create_graphics_device(pickStrategy, id); }
        void destroy_graphics_device(GraphicsDevice graphicsDevice) { g_Backend.__device__destroy_graphics_device(graphicsDevice); }
        GPUVendor get_gpu_vendor(GraphicsDevice device) { return g_Backend.__device__get_gpu_vendor(device); }
        const char* get_device_name(GraphicsDevice device) { return g_Backend.__device__get_device_name(device); }
        bool feature_support(GraphicsDevice device, GPUFeature feature) { return g_Backend.__device__feature_support(device, feature); }
        CoopMatTier coop_mat_tier(GraphicsDevice device) { return g_Backend.__device__coop_mat_tier(device); }
        void set_stable_power_state(GraphicsDevice device, bool state) { g_Backend.__device__set_stable_power_state(device, state); }
    }

    namespace command_queue
    {
        CommandQueue create_command_queue(GraphicsDevice graphicsDevice, CommandQueuePriority directPriority, CommandQueuePriority computePriority, CommandQueuePriority copyPriority) { return g_Backend.__command_queue__create_command_queue(graphicsDevice, directPriority, computePriority, copyPriority); }
        void destroy_command_queue(CommandQueue commandQueue) { g_Backend.__command_queue__destroy_command_queue(commandQueue); }
        void execute_command_buffer(CommandQueue commandQueue, CommandBuffer commandBuffer, bool swapChain) { g_Backend.__command_queue__execute_command_buffer(commandQueue, commandBuffer, swapChain); }
        void signal(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type) { g_Backend.__command_queue__signal(commandQueue, fence, value, type); }
        void wait(CommandQueue commandQueue, Fence fence, uint64_t value, CommandBufferType type) { g_Backend.__command_queue__wait(commandQueue, fence, value, type); }
        void flush(CommandQueue commandQueue, CommandBufferType type) { g_Backend.__command_queue__flush(commandQueue, type); }
    }

    namespace command_buffer
    {
        CommandBuffer create_command_buffer(GraphicsDevice graphicsDevice, CommandBufferType commandBufferType) { return g_Backend.__command_buffer__create_command_buffer(graphicsDevice, commandBufferType); }
        void destroy_command_buffer(CommandBuffer command_buffer) { g_Backend.__command_buffer__destroy_command_buffer(command_buffer); }
        void reset(CommandBuffer commandBuffer) { g_Backend.__command_buffer__reset(commandBuffer); }
        void close(CommandBuffer commandBuffer) { g_Backend.__command_buffer__close(commandBuffer); }
        void clear_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture, const float4& color) { g_Backend.__command_buffer__clear_render_texture(commandBuffer, renderTexture, color); }
        void clear_depth_texture(CommandBuffer commandBuffer, RenderTexture depthTexture, float value) { g_Backend.__command_buffer__clear_depth_texture(commandBuffer, depthTexture, value); }
        void clear_depth_stencil_texture(CommandBuffer commandBuffer, RenderTexture depthTexture, float depth, uint8_t stencil) { g_Backend.__command_buffer__clear_depth_stencil_texture(commandBuffer, depthTexture, depth, stencil); }
        void clear_stencil_texture(CommandBuffer commandBuffer, RenderTexture stencilTexutre, uint8_t stencil) { g_Backend.__command_buffer__clear_stencil_texture(commandBuffer, stencilTexutre, stencil); }
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture) { g_Backend.__command_buffer__set_render_texture_1(commandBuffer, renderTexture); }
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture, RenderTexture depthTexture) { g_Backend.__command_buffer__set_render_texture_2(commandBuffer, renderTexture, depthTexture); }
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture0, RenderTexture renderTexture1, RenderTexture depthTexture) { g_Backend.__command_buffer__set_render_texture_3(commandBuffer, renderTexture0, renderTexture1, depthTexture); }
        void set_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture0, RenderTexture renderTexture1, RenderTexture renderTexture2, RenderTexture depthTexture) { g_Backend.__command_buffer__set_render_texture_4(commandBuffer, renderTexture0, renderTexture1, renderTexture2, depthTexture); }

        void copy_graphics_buffer(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, GraphicsBuffer outputBuffer) { g_Backend.__command_buffer__copy_graphics_buffer_1(commandBuffer, inputBuffer, outputBuffer); }
        void copy_graphics_buffer(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint32_t inputOffset, GraphicsBuffer outputBuffer, uint32_t outputOffset, uint64_t size) { g_Backend.__command_buffer__copy_graphics_buffer_2(commandBuffer, inputBuffer, inputOffset, outputBuffer, outputOffset, size); }
        void upload_constant_buffer(CommandBuffer commandBuffer, ConstantBuffer inputBuffer, ConstantBuffer outputBuffer) { g_Backend.__command_buffer__upload_constant_buffer_1(commandBuffer, inputBuffer, outputBuffer); }
        void upload_constant_buffer(CommandBuffer commandBuffer, ConstantBuffer constantBuffer) { g_Backend.__command_buffer__upload_constant_buffer_2(commandBuffer, constantBuffer); }
        void copy_texture(CommandBuffer commandBuffer, Texture inputTexture, Texture outputTexture) { g_Backend.__command_buffer__copy_texture_1(commandBuffer, inputTexture, outputTexture); }
        void copy_texture(CommandBuffer commandBuffer, RenderTexture inputTexture, uint32_t inputIdx, RenderTexture outputTexture, uint32_t outputIdx) { g_Backend.__command_buffer__copy_texture_2(commandBuffer, inputTexture, inputIdx, outputTexture, outputIdx); }
        void copy_render_texture(CommandBuffer commandBuffer, RenderTexture inputTexture, RenderTexture outputTexture) { g_Backend.__command_buffer__copy_render_texture(commandBuffer, inputTexture, outputTexture); }
        void copy_buffer_into_texture(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, Texture outputTexture, uint32_t sliceIdx, uint32_t mipIdx) { g_Backend.__command_buffer__copy_buffer_into_texture(commandBuffer, inputBuffer, bufferOffset, outputTexture, sliceIdx, mipIdx); }
        void copy_buffer_into_texture_mip(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, Texture outputTexture, uint32_t mipIdx) { g_Backend.__command_buffer__copy_buffer_into_texture_mip(commandBuffer, inputBuffer, bufferOffset, outputTexture, mipIdx); }
        void copy_buffer_into_texture_mips(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, uint32_t imageSize, Texture outputTexture, uint32_t sliceIdx) { g_Backend.__command_buffer__copy_buffer_into_texture_mips(commandBuffer, inputBuffer, bufferOffset, imageSize, outputTexture, sliceIdx); }
        void copy_buffer_into_render_texture(CommandBuffer commandBuffer, GraphicsBuffer inputBuffer, uint64_t bufferOffset, Texture outputRenderTexture, uint32_t sliceIdx) { g_Backend.__command_buffer__copy_buffer_into_render_texture(commandBuffer, inputBuffer, bufferOffset, outputRenderTexture, sliceIdx); }
        void copy_texture_into_buffer(CommandBuffer commandBuffer, Texture inputTexture, uint32_t sliceIdx, uint32_t mipIdx, GraphicsBuffer outputBuffer, uint64_t bufferOffset) { g_Backend.__command_buffer__copy_texture_into_buffer(commandBuffer, inputTexture, sliceIdx, mipIdx, outputBuffer, bufferOffset); }
        void copy_render_texture_into_buffer(CommandBuffer commandBuffer, RenderTexture inputTexture, uint32_t sliceIdx, GraphicsBuffer outputBuffer, uint64_t bufferOffset) { g_Backend.__command_buffer__copy_render_texture_into_buffer(commandBuffer, inputTexture, sliceIdx, outputBuffer, bufferOffset); }
        void uav_barrier_buffer(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer) { g_Backend.__command_buffer__uav_barrier_buffer(commandBuffer, targetBuffer); }
        void uav_barrier_texture(CommandBuffer commandBuffer, Texture texture) { g_Backend.__command_buffer__uav_barrier_texture(commandBuffer, texture); }
        void uav_barrier_render_texture(CommandBuffer commandBuffer, RenderTexture renderTexture) { g_Backend.__command_buffer__uav_barrier_render_texture(commandBuffer, renderTexture); }
        void transition_to_common(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer) { g_Backend.__command_buffer__transition_to_common(commandBuffer, targetBuffer); }
        void transition_to_copy_source(CommandBuffer commandBuffer, GraphicsBuffer targetBuffer) { g_Backend.__command_buffer__transition_to_copy_source(commandBuffer, targetBuffer); }
        void transition_to_present(CommandBuffer commandBuffer, RenderTexture renderTexture) { g_Backend.__command_buffer__transition_to_present(commandBuffer, renderTexture); }
        
        void set_compute_shader_cbuffer(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, ConstantBuffer constantBuffer) { g_Backend.__command_buffer__set_compute_shader_cbuffer(commandBuffer, computeShader, name, constantBuffer); }
        void set_compute_shader_buffer(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, GraphicsBuffer graphicsBuffer) { g_Backend.__command_buffer__set_compute_shader_buffer(commandBuffer, computeShader, name, graphicsBuffer); }
        void set_compute_shader_texture(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, Texture texture, uint32_t mipLevel) { g_Backend.__command_buffer__set_compute_shader_texture(commandBuffer, computeShader, name, texture, mipLevel); }
        void set_compute_shader_render_texture(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, RenderTexture texture) { g_Backend.__command_buffer__set_compute_shader_render_texture(commandBuffer, computeShader, name, texture); }
        void set_compute_shader_sampler(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, Sampler sampler) { g_Backend.__command_buffer__set_compute_shader_sampler(commandBuffer, computeShader, name, sampler); }
        void set_compute_shader_rtas(CommandBuffer commandBuffer, ComputeShader computeShader, const char* name, TopLevelAS rtas) { g_Backend.__command_buffer__set_compute_shader_rtas(commandBuffer, computeShader, name, rtas); }
        void dispatch(CommandBuffer commandBuffer, ComputeShader computeShader, uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ) { g_Backend.__command_buffer__dispatch(commandBuffer, computeShader, sizeX, sizeY, sizeZ); }
        void dispatch_indirect(CommandBuffer commandBuffer, ComputeShader computeShader, GraphicsBuffer indirectBuffer, uint32_t offset) { g_Backend.__command_buffer__dispatch_indirect(commandBuffer, computeShader, indirectBuffer, offset); }
        
        void set_viewport(CommandBuffer commandBuffer, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) { g_Backend.__command_buffer__set_viewport(commandBuffer, offsetX, offsetY, width, height); }
        void set_graphics_pipeline_cbuffer(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, ConstantBuffer constantBuffer) { g_Backend.__command_buffer__set_graphics_pipeline_cbuffer(commandBuffer, graphicsPipeline, name, constantBuffer); }
        void set_graphics_pipeline_buffer(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, GraphicsBuffer graphicsBuffer, uint64_t bufferOffset) { g_Backend.__command_buffer__set_graphics_pipeline_buffer(commandBuffer, graphicsPipeline, name, graphicsBuffer, bufferOffset); }
        void set_graphics_pipeline_texture(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, Texture texture) { g_Backend.__command_buffer__set_graphics_pipeline_texture(commandBuffer, graphicsPipeline, name, texture); }
        void set_graphics_pipeline_render_texture(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, RenderTexture renderTexture) { g_Backend.__command_buffer__set_graphics_pipeline_render_texture(commandBuffer, graphicsPipeline, name, renderTexture); }
        void set_graphics_pipeline_sampler(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, Sampler sampler) { g_Backend.__command_buffer__set_graphics_pipeline_sampler(commandBuffer, graphicsPipeline, name, sampler); }
        void set_graphics_pipeline_rtas(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, const char* name, TopLevelAS rtas) { g_Backend.__command_buffer__set_graphics_pipeline_rtas(commandBuffer, graphicsPipeline, name, rtas); }
        void draw_indexed(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, uint32_t numTriangles, uint32_t numInstances, DrawPrimitive primitive) { g_Backend.__command_buffer__draw_indexed(commandBuffer, graphicsPipeline, vertexBuffer, indexBuffer, numTriangles, numInstances, primitive); }
        void draw_procedural(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, uint32_t numTriangles, uint32_t numInstances, DrawPrimitive primitive) { g_Backend.__command_buffer__draw_procedural(commandBuffer, graphicsPipeline, numTriangles, numInstances, primitive); }
        void draw_procedural_indirect(CommandBuffer commandBuffer, GraphicsPipeline graphicsPipeline, GraphicsBuffer indirectBuffer, uint64_t buffeOffset) { g_Backend.__command_buffer__draw_procedural_indirect(commandBuffer, graphicsPipeline, indirectBuffer, buffeOffset); }

        void build_blas(CommandBuffer cmdB, BottomLevelAS blas) { g_Backend.__command_buffer__build_blas(cmdB, blas); }
        void build_tlas(CommandBuffer cmdB, TopLevelAS tlas) { g_Backend.__command_buffer__build_tlas(cmdB, tlas); }

        void start_section(CommandBuffer commandBuffer, const std::string& eventName) { g_Backend.__command_buffer__start_section(commandBuffer, eventName); }
        void end_section(CommandBuffer commandBuffer) { g_Backend.__command_buffer__end_section(commandBuffer); }

        void enable_profiling_scope(CommandBuffer commandBuffer, ProfilingScope scope) { g_Backend.__command_buffer__enable_profiling_scope(commandBuffer, scope); }
        void disable_profiling_scope(CommandBuffer commandBuffer, ProfilingScope scope) { g_Backend.__command_buffer__disable_profiling_scope(commandBuffer, scope); }

        void convert_mat_32_to_16(CommandBuffer commandBuffer, GraphicsBuffer inputMatrixBuffer, uint64_t inputOffset, GraphicsBuffer outputMatrixBuffer, uint64_t outputOffset, uint32_t width, uint32_t height, bool optimal) { g_Backend.__command_buffer__convert_mat_32_to_16(commandBuffer, inputMatrixBuffer, inputOffset, outputMatrixBuffer, outputOffset, width, height, optimal); }
    }

    namespace window
    {
        RenderWindow create_window(GraphicsDevice device, uint64_t hInstance, uint32_t width, uint32_t height, const char* windowName) { return g_Backend.__window__create_window(device, hInstance, width, height, windowName); }
        void destroy_window(RenderWindow renderWindow) { g_Backend.__window__destroy_window(renderWindow); }
        void viewport_size(RenderWindow window, uint2& size) { g_Backend.__window__viewport_size(window, size); }
        uint2 viewport_center(RenderWindow window) { return g_Backend.__window__viewport_center(window); }
        void viewport_bounds(RenderWindow renderWindow, uint4& bounds) { g_Backend.__window__viewport_bounds(renderWindow, bounds); }
        void window_size(RenderWindow window, uint2& size) { g_Backend.__window__window_size(window, size); }
        uint2 window_center(RenderWindow window) { return g_Backend.__window__window_center(window); }
        void window_bounds(RenderWindow renderWindow, uint4& bounds) { g_Backend.__window__window_bounds(renderWindow, bounds); }
        void handle_messages(RenderWindow renderWindow) { g_Backend.__window__handle_messages(renderWindow); }
        void show(RenderWindow renderWindow) { g_Backend.__window__show(renderWindow); }
        void hide(RenderWindow renderWindow) { g_Backend.__window__hide(renderWindow); }
        void set_cursor_visibility(RenderWindow renderWindow, bool state) { g_Backend.__window__set_cursor_visibility(renderWindow, state); }
        void set_cursor_pos(RenderWindow renderWindow, uint2 position) { g_Backend.__window__set_cursor_pos(renderWindow, position); }
    }

    namespace swap_chain
    {
        SwapChain create_swap_chain(RenderWindow window, GraphicsDevice graphicsDevice, CommandQueue commandQueue, TextureFormat format) { return g_Backend.__swap_chain__create_swap_chain(window, graphicsDevice, commandQueue, format); }
        void destroy_swap_chain(SwapChain swapChain) { g_Backend.__swap_chain__destroy_swap_chain(swapChain); }
        RenderTexture get_current_render_texture(SwapChain swapChain) { return g_Backend.__swap_chain__get_current_render_texture(swapChain); }
        void present(SwapChain swapChain, CommandQueue cmQ) { g_Backend.__swap_chain__present(swapChain, cmQ); }
    }

    namespace resources
    {
        Sampler create_sampler(GraphicsDevice graphicsDevice, const SamplerDescriptor& smplDesc) { return g_Backend.__graphics_resources__create_sampler(graphicsDevice, smplDesc); }
        void destroy_sampler(Sampler sampler) { g_Backend.__graphics_resources__destroy_sampler(sampler); }

        Texture create_texture(GraphicsDevice gd, TextureType type, uint32_t w, uint32_t h, uint32_t d, uint32_t mip, bool uav, TextureFormat fmt, float4 clr, const char* name) { return g_Backend.__graphics_resources__create_texture_1(gd, type, w, h, d, mip, uav, fmt, clr, name); }
        Texture create_texture(GraphicsDevice gd, const TextureDescriptor& desc) { return g_Backend.__graphics_resources__create_texture_2(gd, desc); }
        void destroy_texture(Texture texture) { g_Backend.__graphics_resources__destroy_texture(texture); }
        void texture_dimensions(Texture texture, uint32_t& w, uint32_t& h, uint32_t& d) { g_Backend.__graphics_resources__texture_dimensions(texture, w, h, d); }

        RenderTexture create_render_texture(GraphicsDevice gd, TextureType type, uint32_t w, uint32_t h, uint32_t d, uint32_t mip, bool uav, TextureFormat fmt, float4 clr, const char* name) { return g_Backend.__graphics_resources__create_render_texture_1(gd, type, w, h, d, mip, uav, fmt, clr, name); }
        RenderTexture create_render_texture(GraphicsDevice gd, const TextureDescriptor& desc) { return g_Backend.__graphics_resources__create_render_texture_2(gd, desc); }
        void destroy_render_texture(RenderTexture rt) { g_Backend.__graphics_resources__destroy_render_texture(rt); }
        void render_texture_dimensions(RenderTexture rt, uint32_t& w, uint32_t& h, uint32_t& d) { g_Backend.__graphics_resources__render_texture_dimensions(rt, w, h, d); }

        GraphicsBuffer create_graphics_buffer(GraphicsDevice gd, uint64_t size, uint32_t elemSize, GraphicsBufferType type, uint32_t flags) { return g_Backend.__graphics_resources__create_graphics_buffer(gd, size, elemSize, type, flags); }
        void destroy_graphics_buffer(GraphicsBuffer gb) { g_Backend.__graphics_resources__destroy_graphics_buffer(gb); }
        void set_buffer_data(GraphicsBuffer gb, const char* data, uint64_t size, uint32_t offset) { g_Backend.__graphics_resources__set_buffer_data(gb, data, size, offset); }
        char* allocate_cpu_buffer(GraphicsBuffer gb) { return g_Backend.__graphics_resources__allocate_cpu_buffer(gb); }
        void release_cpu_buffer(GraphicsBuffer gb) { g_Backend.__graphics_resources__release_cpu_buffer(gb); }
        void set_buffer_debug_name(GraphicsBuffer gb, const char* name) { g_Backend.__graphics_resources__set_buffer_debug_name(gb, name); }

        ConstantBuffer create_constant_buffer(GraphicsDevice gd, uint32_t elemSize, ConstantBufferType type) { return g_Backend.__graphics_resources__create_constant_buffer(gd, elemSize, type); }
        void destroy_constant_buffer(ConstantBuffer cb) { g_Backend.__graphics_resources__destroy_constant_buffer(cb); }
        void set_constant_buffer(ConstantBuffer cb, const char* data, uint32_t size) { g_Backend.__graphics_resources__set_constant_buffer(cb, data, size); }

        BottomLevelAS create_blas(GraphicsDevice device, GraphicsBuffer vb, uint32_t vtxCount, GraphicsBuffer ib, uint32_t triCount, uint32_t stride) { return g_Backend.__graphics_resources__create_blas(device, vb, vtxCount, ib, triCount, stride); }
        void destroy_blas(BottomLevelAS blas) { g_Backend.__graphics_resources__destroy_blas(blas); }

        TopLevelAS create_tlas(GraphicsDevice device, uint32_t numBLAS) { return g_Backend.__graphics_resources__create_tlas(device, numBLAS); }
        void destroy_tlas(TopLevelAS tlas) { g_Backend.__graphics_resources__destroy_tlas(tlas); }
        void set_tlas_instance(TopLevelAS tlas, BottomLevelAS blas, uint32_t index) { g_Backend.__graphics_resources__set_tlas_instance(tlas, blas, index); }
        void upload_tlas_instance_data(TopLevelAS tlas) { g_Backend.__graphics_resources__upload_tlas_instance_data(tlas); }
    }

    namespace compute_shader
    {
        ComputeShader create_compute_shader(GraphicsDevice graphicsDevice, const ComputeShaderDescriptor& csd, bool experimental) { return g_Backend.__compute_shader__create_compute_shader(graphicsDevice, csd, experimental); }
        void destroy_compute_shader(ComputeShader computeShader) { g_Backend.__compute_shader__destroy_compute_shader(computeShader); }
    }

    namespace graphics_pipeline
    {
        GraphicsPipeline create_graphics_pipeline(GraphicsDevice graphicsDevice, const GraphicsPipelineDescriptor& graphicsPipelineDescriptor) { return g_Backend.__graphics_pipeline__create_graphics_pipeline(graphicsDevice, graphicsPipelineDescriptor); }
        void destroy_graphics_pipeline(GraphicsPipeline graphicsPipeline) { g_Backend.__graphics_pipeline__destroy_graphics_pipeline(graphicsPipeline); }
        void set_stencil_ref(GraphicsPipeline graphicsPipeline, uint8_t stencilRef) { g_Backend.__graphics_pipeline__set_stencil_ref(graphicsPipeline, stencilRef); }
    }

    namespace profiling_scope
    {
        ProfilingScope create_profiling_scope(GraphicsDevice graphicsDevice) { return g_Backend.__profiling_scope__create_profiling_scope(graphicsDevice); }
        void destroy_profiling_scope(ProfilingScope profilingScope) { g_Backend.__profiling_scope__destroy_profiling_scope(profilingScope); }
        uint64_t get_duration_us(ProfilingScope profilingScope, CommandQueue cmdQ, CommandBufferType type) { return g_Backend.__profiling_scope__get_duration_us(profilingScope, cmdQ, type); };
    }

    namespace imgui
    {
        bool initialize_imgui(GraphicsDevice device, RenderWindow window, TextureFormat format) { return g_Backend.__imgui__initialize_imgui(device, window, format); }
        void release_imgui() { g_Backend.__imgui__release_imgui(); }
        void start_frame() { g_Backend.__imgui__start_frame(); }
        void end_frame() { g_Backend.__imgui__end_frame(); }
        void draw_frame(CommandBuffer cmd, RenderTexture renderTexture) { g_Backend.__imgui__draw_frame(cmd, renderTexture); }
        void handle_input(RenderWindow window, const EventData& data) { g_Backend.__imgui__handle_input(window, data); }
    }
}