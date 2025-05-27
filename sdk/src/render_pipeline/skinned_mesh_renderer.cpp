/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "render_pipeline/skinned_mesh_renderer.h"
#include "graphics/backend.h"
#include "math/operators.h"
#include "tools/shader_utils.h"
#include "tools/dirent.h"
#include "imgui/imgui.h"

SkinnedMeshRenderer::SkinnedMeshRenderer()
{
}

SkinnedMeshRenderer::~SkinnedMeshRenderer()
{
}

void SkinnedMeshRenderer::initialize(GraphicsDevice device, const std::string& modelName)
{
    //Keep track of the device
	m_Device = device;

    // Import the animation
    mesh::import_mesh_animation(modelName.c_str(), m_AnimMesh);

    // Set up the animation data
    m_NumFrames = (uint32_t)m_AnimMesh.vertexBufferArray.size();
    m_NumTriangles = (uint32_t)m_AnimMesh.indexBuffer.size();
    m_NumVertices = (uint32_t)m_AnimMesh.vertexBufferArray[0].data.size();
    m_AnimVertexBuffer.resize(m_NumFrames);
    m_ActiveAnimation = false;
    m_AnimationSpeed = 0.0;

    // Allocate the runtime buffers
    m_AnimIndexBuffer = graphics::resources::create_graphics_buffer(m_Device, m_NumTriangles * sizeof(uint3), sizeof(uint32_t), GraphicsBufferType::Default);
    for (uint32_t idx = 0; idx < m_NumFrames; ++idx)
        m_AnimVertexBuffer[idx] = graphics::resources::create_graphics_buffer(m_Device, m_NumVertices * sizeof(VertexData), sizeof(VertexData), GraphicsBufferType::Default);
    m_SkinnedVertexBuffer = graphics::resources::create_graphics_buffer(m_Device, m_NumVertices * sizeof(VertexData), sizeof(VertexData), GraphicsBufferType::Default);
    m_DisplacementBuffer = graphics::resources::create_graphics_buffer(m_Device, 4 * sizeof(float), sizeof(float), GraphicsBufferType::Default);

    // Ray tracing data
    m_BLAS = graphics::resources::create_blas(m_Device, m_SkinnedVertexBuffer, m_NumVertices, m_AnimIndexBuffer, m_NumTriangles, sizeof(VertexData));
    m_TLAS = graphics::resources::create_tlas(m_Device, 1);
    graphics::resources::set_tlas_instance(m_TLAS, m_BLAS, 0);
    graphics::resources::upload_tlas_instance_data(m_TLAS);
}

void SkinnedMeshRenderer::release()
{
    // Buffers
    graphics::resources::destroy_graphics_buffer(m_AnimIndexBuffer);
    graphics::resources::destroy_graphics_buffer(m_SkinnedVertexBuffer);
    graphics::resources::destroy_graphics_buffer(m_DisplacementBuffer);
    for (uint32_t idx = 0; idx < m_NumFrames; ++idx)
        graphics::resources::destroy_graphics_buffer(m_AnimVertexBuffer[idx]);

    // Shaders
    graphics::compute_shader::destroy_compute_shader(m_SkinCS);
    graphics::compute_shader::destroy_compute_shader(m_DisplEvalCS);
    graphics::graphics_pipeline::destroy_graphics_pipeline(m_VisibilityPassGP);

    // Ray tracing data
    graphics::resources::destroy_blas(m_BLAS);
    graphics::resources::destroy_tlas(m_TLAS);
}

// Resource loading
void SkinnedMeshRenderer::reload_shaders(const std::string& shaderLibrary)
{
    // Skinning
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Mesh\\SkinMesh.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_SkinCS);
    }

    // Displacement evaluation
    {
        ComputeShaderDescriptor csd;
        csd.includeDirectories.push_back(shaderLibrary);
        csd.filename = shaderLibrary + "\\Mesh\\DisplacementEvaluation.compute";
        compile_and_replace_compute_shader(m_Device, csd, m_DisplEvalCS);
    }

    // Visibility pass
    {
        GraphicsPipelineDescriptor gpd;
        gpd.includeDirectories.push_back(shaderLibrary);
        gpd.filename = shaderLibrary + "\\Mesh\\VisibilityPass.graphics";
        gpd.rtFormat[0] = TextureFormat::R16_UInt;
        gpd.depthStencilState.enableDepth = true;
        gpd.depthStencilState.depthtest = DepthTest::LEqual;
        gpd.depthStencilState.depthWrite = true;
        gpd.depthStencilState.depthStencilFormat = TextureFormat::Depth32Stencil8;
        gpd.cullMode = CullMode::Back;
        compile_and_replace_graphics_pipeline(m_Device, gpd, m_VisibilityPassGP);
    }
}

void upload_index_buffer(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const std::vector<uint3>& indexBufferData, GraphicsBuffer indexBuffer)
{
    // Create the upload index buffer
    GraphicsBuffer indexBufferUp = graphics::resources::create_graphics_buffer(device, indexBufferData.size() * sizeof(uint3), sizeof(uint32_t), GraphicsBufferType::Upload);
    graphics::resources::set_buffer_data(indexBufferUp, (char*)indexBufferData.data(), indexBufferData.size() * sizeof(uint3));

    // Copy
    graphics::command_buffer::reset(cmdB);
    graphics::command_buffer::copy_graphics_buffer(cmdB, indexBufferUp, indexBuffer);
    graphics::command_buffer::close(cmdB);
    graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
    graphics::command_queue::flush(cmdQ);

    // Clear
    graphics::resources::destroy_graphics_buffer(indexBufferUp);
}

void upload_vertex_buffer(GraphicsDevice device, CommandQueue cmdQ, CommandBuffer cmdB, const std::vector<VertexData>& vertexBufferData, GraphicsBuffer vertexBuffer)
{
    // Create the upload vertex buffer
    GraphicsBuffer vertexBufferUp = graphics::resources::create_graphics_buffer(device, vertexBufferData.size() * sizeof(VertexData), sizeof(VertexData), GraphicsBufferType::Upload);
    graphics::resources::set_buffer_data(vertexBufferUp, (char*)vertexBufferData.data(), vertexBufferData.size() * sizeof(VertexData));

    // Reset the command buffer
    graphics::command_buffer::reset(cmdB);

    // Copy the upload buffers
    graphics::command_buffer::copy_graphics_buffer(cmdB, vertexBufferUp, vertexBuffer);

    // Close and flush the command buffer
    graphics::command_buffer::close(cmdB);
    graphics::command_queue::execute_command_buffer(cmdQ, cmdB);
    graphics::command_queue::flush(cmdQ);

    graphics::resources::destroy_graphics_buffer(vertexBufferUp);
}

void SkinnedMeshRenderer::upload_geometry(CommandQueue cmdQ, CommandBuffer cmdB)
{
    // Upload index buffers
    upload_index_buffer(m_Device, cmdQ, cmdB, m_AnimMesh.indexBuffer, m_AnimIndexBuffer);

    // Upload all the vertex buffers
    for (uint32_t idx = 0; idx < m_NumFrames; ++idx)
        upload_vertex_buffer(m_Device, cmdQ, cmdB, m_AnimMesh.vertexBufferArray[idx].data, m_AnimVertexBuffer[idx]);
}

void SkinnedMeshRenderer::update_mesh(CommandBuffer cmdB, ConstantBuffer globalCB)
{
    // Skin the mesh
    uint32_t keyFrame = current_animation_frame();
    uint32_t nextFrame = next_animation_frame();

    // Skinning
    {
        // Constant buffers
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_SkinCS, "_GlobalCB", globalCB);

        // Input buffers
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SkinCS, "_VertexBufferA", m_AnimVertexBuffer[keyFrame]);
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SkinCS, "_VertexBufferB", m_AnimVertexBuffer[nextFrame]);

        // Output buffers
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_SkinCS, "_VertexBufferRW", m_SkinnedVertexBuffer);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch(cmdB, m_SkinCS, (m_NumVertices + 31) / 32, 1, 1);
        graphics::command_buffer::uav_barrier_buffer(cmdB, m_SkinnedVertexBuffer);
    }

    // Displacement Eval
    {
        // Constant buffers
        graphics::command_buffer::set_compute_shader_cbuffer(cmdB, m_DisplEvalCS, "_GlobalCB", globalCB);

        // Input buffers
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_DisplEvalCS, "_SkinnedVertexBuffer", m_SkinnedVertexBuffer);

        // Output buffers
        graphics::command_buffer::set_compute_shader_buffer(cmdB, m_DisplEvalCS, "_DisplacementBuffer", m_DisplacementBuffer);

        // Dispatch + Barrier
        graphics::command_buffer::dispatch(cmdB, m_DisplEvalCS, 1, 1, 1);
        graphics::command_buffer::uav_barrier_buffer(cmdB, m_DisplacementBuffer);
    }

    // Build the ray tracing acceleration structures
    graphics::command_buffer::build_blas(cmdB, m_BLAS);
    graphics::command_buffer::build_tlas(cmdB, m_TLAS);
}

void SkinnedMeshRenderer::render_mesh(CommandBuffer cmdB, ConstantBuffer globalCB, RenderTexture colorBuffer, RenderTexture depthBuffer)
{
    graphics::command_buffer::start_section(cmdB, "Visibility Buffer Pass");
    {
        // Render Target
        graphics::command_buffer::set_render_texture(cmdB, colorBuffer, depthBuffer);

        // Constant buffers
        graphics::command_buffer::set_graphics_pipeline_cbuffer(cmdB, m_VisibilityPassGP, "_GlobalCB", globalCB);

        // Input buffers
        graphics::command_buffer::set_graphics_pipeline_buffer(cmdB, m_VisibilityPassGP, "_VertexBuffer", m_SkinnedVertexBuffer);
        graphics::command_buffer::set_graphics_pipeline_buffer(cmdB, m_VisibilityPassGP, "_IndexBuffer", m_AnimIndexBuffer);

        // Draw
        graphics::command_buffer::draw_procedural(cmdB, m_VisibilityPassGP, m_NumTriangles, 1);
    }
    graphics::command_buffer::end_section(cmdB);
}

void SkinnedMeshRenderer::update(double deltaTime)
{
    // The animation is active, but we are not running it as full speed , we need to speed up to animation speed of 1.0
    if (m_ActiveAnimation && m_AnimationSpeed < 1.0)
        m_AnimationSpeed = std::min(1.0f, m_AnimationSpeed + (float)deltaTime * 2.0f);

    // The animation has been requested to stop, but the speed is not zero
    if (!m_ActiveAnimation && m_AnimationSpeed == 1.0 && !m_SlowingDown)
    {
        // How much time animation time left?
        float timeLeft = (1.0f - m_CurrentTime) * m_Duration;

        // if we have more than  half a second, we trigger the  slow down, otherwise it will have to wait for the next loop
        if (std::abs(timeLeft - 0.25) < 0.01)
            m_SlowingDown = true;
    }

    // The animation has been requested to stop, but the speed is not zero
    if (!m_ActiveAnimation && m_SlowingDown)
    {
        m_AnimationSpeed = std::max(0.0f, m_AnimationSpeed - (float)deltaTime * 2.0f);
        if (m_AnimationSpeed == 0.0)
            m_SlowingDown = false;
    }

    // If the animation is running
    if (m_AnimationSpeed > 0.0)
        m_CurrentTime += m_AnimationSpeed * (float)deltaTime / m_Duration;

    // Adjust the time to be in [0, 1] for frame sampling
    if (m_CurrentTime >= 1.0)
        m_CurrentTime = m_CurrentTime - (uint32_t)m_CurrentTime;
}

GraphicsBuffer SkinnedMeshRenderer::vertex_buffer() const
{
    return m_SkinnedVertexBuffer;
}

GraphicsBuffer SkinnedMeshRenderer::index_buffer() const
{
    return m_AnimIndexBuffer;
}

uint32_t SkinnedMeshRenderer::current_animation_frame() const
{
    return uint32_t(m_CurrentTime * m_NumFrames) % m_NumFrames;
}

uint32_t SkinnedMeshRenderer::next_animation_frame() const
{
    uint32_t current = current_animation_frame();
    return (current + 1) % m_NumFrames;
}

float SkinnedMeshRenderer::interpolation_factor() const
{
    uint32_t current = current_animation_frame();
    return m_CurrentTime * m_NumFrames - current;
}

float SkinnedMeshRenderer::animation_time() const
{
    return m_CurrentTime;
}

void SkinnedMeshRenderer::set_animation_state(bool state)
{
    m_ActiveAnimation = state;
}

void SkinnedMeshRenderer::render_ui()
{
    ImGui::SeparatorText("I spared no expenses");
    ImGui::Checkbox("Hunt", &m_ActiveAnimation);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    float enthusiasm = 1.0f - (m_Duration - 0.5f) / 2.5f;
    ImGui::SliderFloat("Enthusiasm", &enthusiasm, 0.0f, 1.0f);
    m_Duration = lerp(0.5f, 3.0f, 1.0f - enthusiasm);
}