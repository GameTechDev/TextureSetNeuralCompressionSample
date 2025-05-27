/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "graphics/types.h"
#include "scene/mesh.h"

// System includes
#include <string>
#include <vector>

class SkinnedMeshRenderer
{
public:
	// Cst & Dst
	SkinnedMeshRenderer();
	~SkinnedMeshRenderer();

	// Init & Release
	void initialize(GraphicsDevice device, const std::string& modelName);
	void release();

	// Resource loading
	void reload_shaders(const std::string& shaderLibrary);
	void upload_geometry(CommandQueue cmdQ, CommandBuffer cmdB);

	// Rendering
	void render_ui();
	void update_mesh(CommandBuffer cmdB, ConstantBuffer globalCB);
	void render_mesh(CommandBuffer cmdB, ConstantBuffer globalCB, RenderTexture colorTexture, RenderTexture depthBuffer);

	// Update
	void update(double time);
	void set_animation_state(bool state);

	// Accessing resources
	GraphicsBuffer vertex_buffer() const;
	GraphicsBuffer index_buffer() const;
	GraphicsBuffer displacement_buffer() const { return m_DisplacementBuffer; }
	TopLevelAS tlas() const { return m_TLAS; }

	// Animation data
	float interpolation_factor() const;
	float animation_time() const;
	uint32_t num_vertices() const { return m_NumVertices; }

private:
	uint32_t current_animation_frame() const;
	uint32_t next_animation_frame() const;

private:
	// Graphics Device
	GraphicsDevice m_Device = 0;

	// Animation mesh
	uint32_t m_NumFrames = 0;
	MeshAnimation m_AnimMesh = MeshAnimation();
	uint32_t m_NumTriangles = 0;
	uint32_t m_NumVertices = 0;

	// Runtime buffers
	GraphicsBuffer m_AnimIndexBuffer = 0;
	std::vector<GraphicsBuffer> m_AnimVertexBuffer;
	GraphicsBuffer m_SkinnedVertexBuffer = 0;
	GraphicsBuffer m_DisplacementBuffer = 0;

	// Other data
	float m_Duration = 1.9f;
	float m_AnimationSpeed = 1.0f;
	float m_CurrentTime = 0.0;
	bool m_ActiveAnimation = true;
	bool m_SlowingDown = false;

	// Shaders
	GraphicsPipeline m_VisibilityPassGP = 0;
	ComputeShader m_SkinCS = 0;
	ComputeShader m_DisplEvalCS = 0;

	// Ray Tracing data
	TopLevelAS m_TLAS = 0;
	BottomLevelAS m_BLAS = 0;
};	