/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Project includes
#include <render_pipeline/types.h>

#include <render_pipeline/gbuffer_renderer.h>
#include <render_pipeline/material_renderer.h>
#include <render_pipeline/skinned_mesh_renderer.h>
#include <render_pipeline/ibl.h>
#include <render_pipeline/texture_manager.h>
#include <render_pipeline/tile_classifier.h>

#include <tools/profiling_helper.h>
#include <tools/camera_controller.h>
#include <tools/command_line.h>

// System includes
#include <string>
#include <memory>

class DinoRenderer
{
public:
	// Cst & Dst
	DinoRenderer();
	~DinoRenderer();

	// Init & release
	void initialize(uint64_t hInstance, const CommandLineOptions& options);
	void release();

	// Launch the loop
	void render_loop();

private:
	// Shaders
	void reload_shaders();

	// Rendering
	void update_constant_buffers(CommandBuffer cmdB);
	void render_ui(CommandBuffer cmdB, RenderTexture rt);
	void render_frame();

	// Updata
	void update(double deltaTime);

	// Inputs
	void process_key_event(uint32_t keyCode, bool state);

private:
	// Graphics Backend
	GraphicsDevice m_Device = 0;
	RenderWindow m_Window = 0;
	CommandQueue m_CmdQueue = 0;
	SwapChain m_SwapChain = 0;
	CommandBuffer m_CmdBuffer = 0;

	// Project directory
	std::string m_ProjectDir = "";
	bool m_CooperativeVectorsSupported = false;

	// Global rendering properties
	uint2 m_ScreenSizeI = { 0, 0 };
	uint2 m_TileSizeI = { 0, 0 };
	float4 m_ScreenSize = { 0.0, 0.0, 0.0, 0.0 };
	uint32_t m_FrameIndex = 0;
	double m_Time = 0.0;
	std::vector<float> m_DurationArray;
	std::vector<float> m_DrawArray;
	uint32_t m_CurrentDuration = UINT32_MAX;

	// UI parameters
	RenderingMode m_RenderingMode = RenderingMode::Count;
	TextureMode m_TextureMode = TextureMode::Count;
	FilteringMode m_FilteringMode = FilteringMode::Count;
	DebugMode m_DebugMode = DebugMode::Count;
	bool m_DisplayUI = true;
	bool m_UseCooperativeVectors = false;
	bool m_EnableCounters = false;
	bool m_EnableFiltering = true;

	// Rendering resources
	ConstantBuffer m_GlobalCB = 0;
	RenderTexture m_VisibilityBuffer = 0;
	RenderTexture m_DepthTexture = 0;
	RenderTexture m_ColorTexture = 0;
	RenderTexture m_ShadowTexture = 0;
	GraphicsBuffer m_GBuffer = 0;

	// Rendering components
	SkinnedMeshRenderer m_MeshRenderer = SkinnedMeshRenderer();
	IBL m_IBL = IBL();
	TextureManager m_TexManager = TextureManager();
	TileClassifier m_Classifier = TileClassifier();

	// Networks
	TSNC m_TSNC = TSNC();

	// Inference
	GBufferRenderer m_GBufferRenderer = GBufferRenderer();
	MaterialRenderer m_MaterialRenderer = MaterialRenderer();

	// Pipeline
	ComputeShader m_ShadowRTCS = 0;
	ComputeShader m_DebugViewCS = 0;
	GraphicsPipeline m_UberPostGP = 0;

	// Components
	CameraController m_CameraController = CameraController();
	ProfilingHelper m_ProfilingHelper = ProfilingHelper();
};
