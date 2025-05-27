/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "graphics/event_collector.h"

#include "render_pipeline/constant_buffers.h"
#include "render_pipeline/dino_renderer.h"

#include "tools/security.h"
#include "tools/shader_utils.h"
#include "tools/string_utilities.h"
#include "tools/imgui_helpers.h"

#include "imgui/imgui.h"

// System includes
#include <chrono>
#include <iostream>

// Number of frames for our performance path
#define NUM_PROFILING_FRAMES 50
#define FRAME_BUFFER_FORMAT TextureFormat::R16G16B16A16_Float

DinoRenderer::DinoRenderer()
{
}

DinoRenderer::~DinoRenderer()
{
}

void DinoRenderer::initialize(uint64_t hInstance, const CommandLineOptions& options)
{
    // Keep the directory
    m_ProjectDir = options.dataDir;

    // Model library
    const std::string& modelLibrary = m_ProjectDir + "\\models";

    // Geometry library
    const std::string& geometryLibrary = m_ProjectDir + "\\geometry";

    // Texture library
    const std::string& textureLibrary = m_ProjectDir + "\\textures";

    // Path library
    const std::string& pathLibrary = m_ProjectDir + "\\paths";

    // Create the graphics components
    graphics::setup_graphics_api(GraphicsAPI::DX12);
    //d3d12::device::enable_debug_layer();
    graphics::device::enable_experimental_features();
    m_Device = graphics::device::create_graphics_device();
    m_Window = graphics::window::create_window(m_Device, (uint64_t)hInstance, 1920, 1080, "Intel TSNC (DX12)");
    m_CmdQueue = graphics::command_queue::create_command_queue(m_Device);
    m_SwapChain = graphics::swap_chain::create_swap_chain(m_Window, m_Device, m_CmdQueue, FRAME_BUFFER_FORMAT);
    m_CmdBuffer = graphics::command_buffer::create_command_buffer(m_Device);

    // Coop vector support
    m_CooperativeVectorsSupported = graphics::device::feature_support(m_Device, GPUFeature::CoopVector);

    // Imgui Init
    graphics::imgui::initialize_imgui(m_Device, m_Window, FRAME_BUFFER_FORMAT);

    // Evaluate the sizes
    uint2 screenSize;
    graphics::window::viewport_size(m_Window, screenSize);
    m_ScreenSizeI = screenSize;
    m_TileSizeI = { m_ScreenSizeI.x / 8, m_ScreenSizeI.y / 4 };
    m_ScreenSize = float4({ (float)m_ScreenSizeI.x, (float)m_ScreenSizeI.y, 1.0f / m_ScreenSizeI.x, 1.0f / m_ScreenSizeI.y });

    // Camera controls
    m_CameraController.initialize(m_Window, m_ScreenSizeI.x, m_ScreenSizeI.y, 35.0 * DEG_TO_RAD, pathLibrary);
    m_CameraController.move_to_poi(options.initialPOI);

    // Initial setup
    m_RenderingMode = RenderingMode::GBufferDeferred;
    m_TextureMode = TextureMode::Neural;
    m_DebugMode = DebugMode::TileInfo;
    m_FilteringMode = FilteringMode::Linear;
    m_DisplayUI = true;
    m_UseCooperativeVectors = m_CooperativeVectorsSupported ? options.enableCooperative : false;
    m_EnableCounters = false;
    m_EnableFiltering = true;
    m_DurationArray.resize(NUM_PROFILING_FRAMES, 0.0f);
    m_DrawArray.resize(NUM_PROFILING_FRAMES, 0.0f);
    m_CurrentDuration = 0;

    // Constant buffer
    m_GlobalCB = graphics::resources::create_constant_buffer(m_Device, sizeof(GlobalCB), ConstantBufferType::Mixed);

    // Render textures
    {
        // Common properties
        TextureDescriptor descriptor;
        descriptor.type = TextureType::Tex2D;
        descriptor.width = m_ScreenSizeI.x;
        descriptor.height = m_ScreenSizeI.y;
        descriptor.depth = 1;
        descriptor.mipCount = 1;

        // Depth buffer
        descriptor.isUAV = false;
        descriptor.format = TextureFormat::Depth32Stencil8;
        descriptor.clearColor = float4({ 1.0f, 0.0f, 0.0f, 0.0f });
        descriptor.debugName = "Depth Texture";
        m_DepthTexture = graphics::resources::create_render_texture(m_Device, descriptor);

        // Visibility buffer
        descriptor.isUAV = true;
        descriptor.format = TextureFormat::R32_UInt;
        descriptor.clearColor = float4({ 0.0f, 0.0f, 0.0f, 1.0f });
        descriptor.debugName = "Visibility Buffer";
        m_VisibilityBuffer = graphics::resources::create_render_texture(m_Device, descriptor);

        // Shadow texture
        descriptor.isUAV = true;
        descriptor.format = TextureFormat::R8_UNorm;
        descriptor.clearColor = float4({ 0.0f, 0.0f, 0.0f, 0.0f });
        descriptor.debugName = "Shadow Texture";
        m_ShadowTexture = graphics::resources::create_render_texture(m_Device, descriptor);

        // Color texture
        descriptor.isUAV = true;
        descriptor.format = FRAME_BUFFER_FORMAT;
        descriptor.clearColor = float4({ 0.5f, 0.5f, 0.5f, 1.0f });
        descriptor.debugName = "Color Texture";
        m_ColorTexture = graphics::resources::create_render_texture(m_Device, descriptor);
    }

    // Components
    m_TSNC.initialize(m_Device, m_CooperativeVectorsSupported);
    m_GBufferRenderer.initialize(m_Device, m_CooperativeVectorsSupported);
    m_MaterialRenderer.initialize(m_Device, m_CooperativeVectorsSupported);
    m_MeshRenderer.initialize(m_Device, geometryLibrary + "\\michel.anim");
    m_IBL.initialize(m_Device, textureLibrary);
    m_TexManager.initialize(m_Device);
    m_Classifier.initialize(m_Device, m_TileSizeI, 1);

    // Load the models
    m_TSNC.reload_network((modelLibrary + "\\michel\\bc1_mip"), 1);

    // Load the shaders
    reload_shaders();

    // Upload to the GPU
    m_TSNC.upload_network(m_CmdQueue, m_CmdBuffer);
    m_MeshRenderer.upload_geometry(m_CmdQueue, m_CmdBuffer);
    m_IBL.upload_textures(m_CmdQueue, m_CmdBuffer);
    m_TexManager.upload_textures(m_CmdQueue, m_CmdBuffer, modelLibrary, "michel");

    // Tools
    m_ProfilingHelper.initialize(m_Device, m_CmdQueue, 2);

    // Allocate the intermediate graphics buffers
    const uint32_t numPixels = m_ScreenSizeI.x * m_ScreenSizeI.y;
    const uint32_t numChannels = m_TSNC.texture_size().z;
    m_GBuffer = graphics::resources::create_graphics_buffer(m_Device, numPixels * sizeof(uint16_t) * numChannels, sizeof(uint16_t), GraphicsBufferType::Default);

    // Post setups
    m_MeshRenderer.set_animation_state(options.enableAnimation);
}

void DinoRenderer::reload_shaders()
{
    // Model library
    std::string shaderLibrary = m_ProjectDir;
    shaderLibrary += "\\shaders";

    // Shadows
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Lighting\\ShadowRT.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_ShadowRTCS);
    }

    // Debug view
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Lighting\\DebugView.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_DebugViewCS);
    }

    // Post process
    {
        GraphicsPipelineDescriptor gpd;
        gpd.filename = shaderLibrary + "\\PostProcess.graphics";
        gpd.includeDirectories.push_back(shaderLibrary);
        gpd.isProcedural = true;
        gpd.rtFormat[0] = TextureFormat::R16G16B16A16_Float;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_UberPostGP);
    }

    // Components
    m_TSNC.reload_shaders(shaderLibrary);
    m_GBufferRenderer.reload_shaders(shaderLibrary, m_TSNC.shader_defines());
    m_MaterialRenderer.reload_shaders(shaderLibrary, m_TSNC);
    m_MeshRenderer.reload_shaders(shaderLibrary);
    m_IBL.reload_shaders(shaderLibrary);
    m_Classifier.reload_shaders(shaderLibrary);
}

void DinoRenderer::release()
{
    // Constant buffer
    graphics::resources::destroy_constant_buffer(m_GlobalCB);

    // Render textures
    graphics::resources::destroy_render_texture(m_VisibilityBuffer);
    graphics::resources::destroy_render_texture(m_DepthTexture);
    graphics::resources::destroy_render_texture(m_ColorTexture);
    graphics::resources::destroy_render_texture(m_ShadowTexture);

    // Buffers
    graphics::resources::destroy_graphics_buffer(m_GBuffer);
    
    // Shaders
    graphics::compute_shader::destroy_compute_shader(m_ShadowRTCS);
    graphics::compute_shader::destroy_compute_shader(m_DebugViewCS);
    graphics::graphics_pipeline::destroy_graphics_pipeline(m_UberPostGP);

    // Components
    m_TSNC.release();
    m_GBufferRenderer.release();
    m_MaterialRenderer.release();
    m_MeshRenderer.release();
    m_IBL.release();
    m_TexManager.release();
    m_ProfilingHelper.release();
    m_Classifier.release();

    // Imgui
    graphics::imgui::release_imgui();

    // Rendering components
    graphics::command_buffer::destroy_command_buffer(m_CmdBuffer);
    graphics::swap_chain::destroy_swap_chain(m_SwapChain);
    graphics::command_queue::destroy_command_queue(m_CmdQueue);
    graphics::window::destroy_window(m_Window);
    graphics::device::destroy_graphics_device(m_Device);
}

void DinoRenderer::render_ui(CommandBuffer cmdB, RenderTexture rt)
{
    if (!m_DisplayUI)
        return;

    // Start
    graphics::imgui::start_frame();

    // Display the UI
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(520.0f, 350.0f));
    ImGui::Begin("Debug Window");
    {
        // Device name
        ImGui::Text(graphics::device::get_device_name(m_Device));

        // Rendering mode
        const char* rendering_mode_labels[] = {"Visibility + Material", "Visibility + GBuffer + Deferred", "Debug"};
        imgui_dropdown_enum<RenderingMode>(m_RenderingMode, "Rendering Mode", rendering_mode_labels);

        // Texture mode
        const char* texture_mode_labels[] = { "Uncompressed", "BC6", "Neural"};
        imgui_dropdown_enum<TextureMode>(m_TextureMode, "Texture Mode", texture_mode_labels);

        // Filtering mode
        const char* filtering_mode_labels[] = { "Nearest", "Linear", "Anisotropic" };
        imgui_dropdown_enum<FilteringMode>(m_FilteringMode, "Filtering Mode", filtering_mode_labels);

        if (m_TextureMode == TextureMode::Neural)
            ImGui::Checkbox("Use Cooperative Vectors", &m_UseCooperativeVectors);

        if (m_TextureMode == TextureMode::Neural && m_UseCooperativeVectors && !m_CooperativeVectorsSupported)
            ImGui::Text("The current DX12 device doesn't support cooperative vectors.");

        // Lighting mode
        if (m_RenderingMode == RenderingMode::Debug)
        {
            const char* debug_mode_labels[] = { "Thickness", "Mask", "Displacement", "Metalness", "Roughness", "AmbientOcclusion", "Normal", "DiffuseColor", "TileInfo"};
            imgui_dropdown_enum<DebugMode>(m_DebugMode, "Debug Mode", debug_mode_labels);
        }

        // Mesh renderer
        m_MeshRenderer.render_ui();

        // Camera controller
        m_CameraController.render_ui();

        ImGui::SeparatorText("Interactions");
        ImGui::Text("Mouse Right Button: Camera interaction.");
        ImGui::Text("F5: Recompile shaders.");
        ImGui::Text("F6: Performance counters view.");
        ImGui::Text("F11: Toggle UI.");
    }
    ImGui::End();

    // Performance window
    if (m_EnableCounters)
    {
        // The max duration
        float maxV = -FLT_MAX;
        for (uint32_t idx = 0; idx < NUM_PROFILING_FRAMES; ++idx)
            maxV = std::max(maxV, m_DurationArray[idx]);

        for (int32_t idx = 0; idx < NUM_PROFILING_FRAMES; ++idx)
        {
            int32_t candIdx = idx - m_CurrentDuration + NUM_PROFILING_FRAMES - 1;
            if (candIdx < 0)
                candIdx += NUM_PROFILING_FRAMES;
            if (candIdx > NUM_PROFILING_FRAMES - 1)
                candIdx -= NUM_PROFILING_FRAMES;
            m_DrawArray[candIdx] = m_DurationArray[idx];
        }

        ImGui::SetNextWindowPos(ImVec2(1620, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 180.0f));
        ImGui::Begin("Peformance Window");

        std::string label = "Current pass time ";
        label += to_string_with_precision(m_DurationArray[m_CurrentDuration], 3) + "(ms)";
        ImGui::PlotHistogram("##Histogram", m_DrawArray.data(), (uint32_t)m_DrawArray.size(), 0, label.c_str(), 0.0f, 1.5f * maxV, ImVec2(285, 145));
        ImGui::End();


    }

    // End imgui
    graphics::imgui::end_frame();
    graphics::imgui::draw_frame(cmdB, rt);
}

void DinoRenderer::update_constant_buffers(CommandBuffer cmdB)
{
    // Grab the camera
    const Camera& camera = m_CameraController.get_camera();

    // Set all the properties
    GlobalCB globalCB;
    globalCB._ViewProjectionMatrix = camera.viewProjection;
    globalCB._InvViewProjectionMatrix = camera.invViewProjection;
    globalCB._CameraPosition = camera.position;
    globalCB._ScreenSize = m_ScreenSizeI;
    globalCB._TextureSize = { m_TSNC.texture_size().x, m_TSNC.texture_size().y };
    globalCB._NumTextureLOD = { log2f((float)globalCB._TextureSize.x), log2f((float)globalCB._TextureSize.x / 2) };
    globalCB._TileSize = m_TileSizeI;
    globalCB._ChannelSet = (uint32_t)m_DebugMode;
    globalCB._AnimationFactor = m_MeshRenderer.interpolation_factor();
    globalCB._AnimationTime = m_MeshRenderer.animation_time();
    globalCB._MeshNumVerts = m_MeshRenderer.num_vertices();
    globalCB._EnablePP = m_RenderingMode != RenderingMode::Debug ? 1.0f : 0.0f;
    globalCB._EnableFiltering = m_EnableFiltering ? 15.0f : 0.0f;
    globalCB._FrameIndex = m_FrameIndex;
    globalCB._SunDirection = float3({ 0.57735026919, 0.57735026919 , 0.57735026919 });

    // Only one MLP for this application
    globalCB._MLPCount = 1;

    // Set and upload
    graphics::resources::set_constant_buffer(m_GlobalCB, (const char*)&globalCB, sizeof(GlobalCB));
    graphics::command_buffer::upload_constant_buffer(cmdB, m_GlobalCB);
}

void DinoRenderer::render_frame()
{
    // Reset the command buffer
    graphics::command_buffer::reset(m_CmdBuffer);
    if (m_EnableCounters)
        m_ProfilingHelper.start_profiling(m_CmdBuffer, 0);

    // Update the constant buffers
    update_constant_buffers(m_CmdBuffer);

    // Update the skinning
    m_MeshRenderer.update_mesh(m_CmdBuffer, m_GlobalCB);

    // Clear the render textures
    graphics::command_buffer::start_section(m_CmdBuffer, "Clear targets");
    {
        graphics::command_buffer::clear_render_texture(m_CmdBuffer, m_VisibilityBuffer, float4({ 0.0, 0.0, 0.0, 1.0 }));
        if (m_RenderingMode == RenderingMode::Debug)
            graphics::command_buffer::clear_render_texture(m_CmdBuffer, m_ColorTexture, float4({ 0.5, 0.5, 0.5, 1.0 }));
        graphics::command_buffer::clear_depth_texture(m_CmdBuffer, m_DepthTexture, 1.0f);
    }
    graphics::command_buffer::end_section(m_CmdBuffer);
    

    // Set the viewport for the frame
    graphics::command_buffer::set_viewport(m_CmdBuffer, 0, 0, m_ScreenSizeI.x, m_ScreenSizeI.y);

    // Render the visibility buffer
    m_MeshRenderer.render_mesh(m_CmdBuffer, m_GlobalCB, m_VisibilityBuffer, m_DepthTexture);

    // Render the shadows
    graphics::command_buffer::start_section(m_CmdBuffer, "Trace shadows");
    {
        // CBVs
        graphics::command_buffer::set_compute_shader_cbuffer(m_CmdBuffer, m_ShadowRTCS, "_GlobalCB", m_GlobalCB);

        // SRVs
        graphics::command_buffer::set_compute_shader_render_texture(m_CmdBuffer, m_ShadowRTCS, "_VisibilityBuffer", m_VisibilityBuffer);
        graphics::command_buffer::set_compute_shader_buffer(m_CmdBuffer, m_ShadowRTCS, "_VertexBuffer", m_MeshRenderer.vertex_buffer());
        graphics::command_buffer::set_compute_shader_buffer(m_CmdBuffer, m_ShadowRTCS, "_IndexBuffer", m_MeshRenderer.index_buffer());
        graphics::command_buffer::set_compute_shader_rtas(m_CmdBuffer, m_ShadowRTCS, "_SceneRTAS", m_MeshRenderer.tlas());

        // UAVs
        graphics::command_buffer::set_compute_shader_render_texture(m_CmdBuffer, m_ShadowRTCS, "_ShadowTextureRW", m_ShadowTexture);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch(m_CmdBuffer, m_ShadowRTCS, m_TileSizeI.x, m_TileSizeI.y, 1);
        graphics::command_buffer::uav_barrier_render_texture(m_CmdBuffer, m_ShadowTexture);
    }
    graphics::command_buffer::end_section(m_CmdBuffer);

    // Classification
    m_Classifier.classify(m_CmdBuffer, m_GlobalCB, m_VisibilityBuffer, m_MeshRenderer.vertex_buffer(), m_MeshRenderer.index_buffer());

    // Trigger the right rendering path
    switch (m_RenderingMode)
    {
        case RenderingMode::GBufferDeferred:
        case RenderingMode::Debug:
        {
            // Depending on if it's the neural path or the other path
            if (m_TextureMode == TextureMode::Neural)
            {
                if (m_EnableCounters)
                    m_ProfilingHelper.start_profiling(m_CmdBuffer, 1);

                m_GBufferRenderer.evaluate_neural_cmp_indirect(m_CmdBuffer, m_GlobalCB,
                    m_VisibilityBuffer, m_MeshRenderer.vertex_buffer(), m_MeshRenderer.index_buffer(), m_GBuffer,
                    m_Classifier, m_UseCooperativeVectors, m_TSNC, m_FilteringMode);

                if (m_EnableCounters)
                    m_ProfilingHelper.end_profiling(m_CmdBuffer, 1);
            }
            else
            {
                // Grab the right texture set
                const TextureSet& texSet = m_TexManager.texture_set(m_TextureMode == TextureMode::BC6H);

                //  GBuffer generation
                if (m_EnableCounters)
                    m_ProfilingHelper.start_profiling(m_CmdBuffer, 1);
                m_GBufferRenderer.evaluate_indirect(m_CmdBuffer, m_GlobalCB, m_VisibilityBuffer, m_Classifier.active_tiles_buffer(), m_Classifier.indirect_buffer(), m_GBuffer, texSet, m_MeshRenderer.vertex_buffer(), m_MeshRenderer.index_buffer(), m_FilteringMode);
                if (m_EnableCounters)
                    m_ProfilingHelper.end_profiling(m_CmdBuffer, 1);
            }

            switch (m_RenderingMode)
            {
                case RenderingMode::GBufferDeferred:
                {
                    // First render the background
                    m_IBL.render_cubemap(m_CmdBuffer, m_GlobalCB, m_ColorTexture, m_ShadowTexture, m_MeshRenderer.displacement_buffer());

                    // Render the lighting
                    m_GBufferRenderer.lighting_indirect(m_CmdBuffer, m_GlobalCB, m_MeshRenderer.vertex_buffer(), m_MeshRenderer.index_buffer(), m_IBL, m_GBuffer, m_Classifier.active_tiles_buffer(), m_Classifier.indirect_buffer(), m_VisibilityBuffer, m_ShadowTexture, m_ColorTexture);
                }
                break;
                case RenderingMode::Debug:
                {
                    // CBVs
                    graphics::command_buffer::set_compute_shader_cbuffer(m_CmdBuffer, m_DebugViewCS, "_GlobalCB", m_GlobalCB);

                    // SRVs
                    graphics::command_buffer::set_compute_shader_render_texture(m_CmdBuffer, m_DebugViewCS, "_VisibilityBuffer", m_VisibilityBuffer);
                    graphics::command_buffer::set_compute_shader_buffer(m_CmdBuffer, m_DebugViewCS, "_InferenceBuffer", m_GBuffer);
                    graphics::command_buffer::set_compute_shader_buffer(m_CmdBuffer, m_DebugViewCS, "_IndexationBuffer", m_Classifier.active_tiles_buffer());

                    // UAVs
                    graphics::command_buffer::set_compute_shader_render_texture(m_CmdBuffer, m_DebugViewCS, "_ColorTextureRW", m_ColorTexture);

                    // Dispatch + Barrier
                    graphics::command_buffer::dispatch_indirect(m_CmdBuffer, m_DebugViewCS, m_Classifier.indirect_buffer());
                    graphics::command_buffer::uav_barrier_render_texture(m_CmdBuffer, m_ColorTexture);
                }
                break;
            }
        }
        break;
        case RenderingMode::MaterialPass:
        {
            // Render the background
            m_IBL.render_cubemap(m_CmdBuffer, m_GlobalCB, m_ColorTexture, m_ShadowTexture, m_MeshRenderer.displacement_buffer());

            // Depending on if it's the neural path or the other path
            if (m_TextureMode == TextureMode::Neural)
            {
                if (m_EnableCounters)
                    m_ProfilingHelper.start_profiling(m_CmdBuffer, 1);
                {
                    m_MaterialRenderer.evaluate_neural_cmp_indirect(m_CmdBuffer, m_GlobalCB, m_TSNC, m_MeshRenderer.vertex_buffer(), m_MeshRenderer.index_buffer(),
                        m_IBL, m_UseCooperativeVectors, m_FilteringMode,
                        m_VisibilityBuffer, m_ShadowTexture, m_Classifier, m_ColorTexture);
                }

                if (m_EnableCounters)
                    m_ProfilingHelper.end_profiling(m_CmdBuffer, 1);
            }
            else
            {
                // Grab the right texture set
                const TextureSet& texSet = m_TexManager.texture_set(m_TextureMode == TextureMode::BC6H);

                // GBuffer generation
                if (m_EnableCounters)
                    m_ProfilingHelper.start_profiling(m_CmdBuffer, 1);
                m_MaterialRenderer.evaluate_indirect(m_CmdBuffer, m_GlobalCB, m_MeshRenderer.vertex_buffer(), m_MeshRenderer.index_buffer(), m_IBL, texSet, m_FilteringMode, m_VisibilityBuffer,
                    m_ShadowTexture, m_Classifier.active_tiles_buffer(), m_Classifier.indirect_buffer(), m_ColorTexture);
                if (m_EnableCounters)
                    m_ProfilingHelper.end_profiling(m_CmdBuffer, 1);
            }
        }
        break;
    }
    if (m_EnableCounters)
        m_ProfilingHelper.end_profiling(m_CmdBuffer, 0);

    // Grab the current swap chain render target
    RenderTexture rTexture = graphics::swap_chain::get_current_render_texture(m_SwapChain);
    
    // Post process
    graphics::command_buffer::start_section(m_CmdBuffer, "Post process");
    {
        graphics::command_buffer::set_viewport(m_CmdBuffer, 0, 0, m_ScreenSizeI.x, m_ScreenSizeI.y);
        graphics::command_buffer::set_render_texture(m_CmdBuffer, rTexture);
        graphics::command_buffer::set_graphics_pipeline_cbuffer(m_CmdBuffer, m_UberPostGP, "_GlobalCB", m_GlobalCB);
        graphics::command_buffer::set_graphics_pipeline_render_texture(m_CmdBuffer, m_UberPostGP, "_ColorTextureIn", m_ColorTexture);
        graphics::command_buffer::draw_procedural(m_CmdBuffer, m_UberPostGP, 1, 1);
    }
    graphics::command_buffer::end_section(m_CmdBuffer);

    // Render UI
    render_ui(m_CmdBuffer, rTexture);

    // Set the render target in present mode
    graphics::command_buffer::transition_to_present(m_CmdBuffer, rTexture);

    // Close the command buffer
    graphics::command_buffer::close(m_CmdBuffer);

    // Execute the command buffer in the command queue
    graphics::command_queue::execute_command_buffer(m_CmdQueue, m_CmdBuffer);

    // Present
    graphics::swap_chain::present(m_SwapChain, m_CmdQueue);

    // Flush the queue
    graphics::command_queue::flush(m_CmdQueue);
}


void DinoRenderer::process_key_event(uint32_t keyCode, bool state)
{
    switch (keyCode)
    {
        case 0x74: // F5
            if (state)
                reload_shaders();
            break;
        case 0x75: // F6
            if (state)
                m_EnableCounters = !m_EnableCounters;
            break;
        case 0x7A: // F11
            if (state)
                m_DisplayUI = !m_DisplayUI;
            break;
    }

    // Propagate to the camera controller
    m_CameraController.process_key_event(keyCode, state);
}

void DinoRenderer::render_loop()
{
    // Render loop
    bool activeLoop = true;
    float lastUpdate = FLT_MAX;
    while (activeLoop)
    {
        auto start = std::chrono::high_resolution_clock::now();

        // Handle the messages
        graphics::window::handle_messages(m_Window);
        uint2 windowCenter = graphics::window::window_center(m_Window);

        // Process the events
        bool resetCursorToCenter = false;
        EventData eventData;
        while (event_collector::peek_event(eventData))
        {
            switch (eventData.type)
            {
                case FrameEvent::Raw:
                    graphics::imgui::handle_input(m_Window, eventData);
                break;
                case FrameEvent::MouseMovement:
                    resetCursorToCenter |= m_CameraController.process_mouse_movement({ (int)eventData.data0, (int)eventData.data1 }, windowCenter, m_ScreenSize);
                    break;
                case FrameEvent::MouseWheel:
                    m_CameraController.process_mouse_wheel((int)eventData.data0);
                    break;
                case FrameEvent::MouseButton:
                    resetCursorToCenter |= m_CameraController.process_mouse_button((MouseButton)eventData.data0, eventData.data1 != 0);
                    break;
                case FrameEvent::KeyDown:
                    process_key_event(eventData.data0, true);
                    break;
                case FrameEvent::KeyUp:
                    process_key_event(eventData.data0, false);
                    break;
                case FrameEvent::Close:
                case FrameEvent::Destroy:
                    activeLoop = false;
                break;
            }
        }

        if (resetCursorToCenter)
        {
            m_FrameIndex = 0;
            graphics::window::set_cursor_pos(m_Window, windowCenter);
        }

        // Draw if needed
        if (event_collector::active_draw_request())
        {
            render_frame();
            m_FrameIndex++;
            event_collector::draw_done();
        }

        // Query the time
        if (m_EnableCounters && lastUpdate > 0.1)
        {
            m_ProfilingHelper.process_scopes(m_CmdQueue);
            float passDurationMS = m_ProfilingHelper.get_scope_last_duration(1) / 1e3f;

            // Move to the next time
            m_CurrentDuration++;
            m_CurrentDuration = m_CurrentDuration % NUM_PROFILING_FRAMES;

            // Save it
            m_DurationArray[m_CurrentDuration] = passDurationMS;
            lastUpdate = 0.0;
        }

        // Evaluate the time
        auto stop = std::chrono::high_resolution_clock::now();
        std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        double deltaTime = duration.count() / 1e9;

        // Update the system
        update(deltaTime);
        lastUpdate += (float)deltaTime;
    }
}

void DinoRenderer::update(double deltaTime)
{
    // Add to the time
    m_Time += deltaTime;

    // Update the controller
    m_CameraController.update(deltaTime);

    // Update the animation
    m_MeshRenderer.update(deltaTime);
}