/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// SDK includes
#include "graphics/descriptors.h"
#include "tools/security.h"

// DX12 includes
#define NOMINMAX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <dxcapi.h>

// System includes
#include <vector>
#include <string>
#include <map>

namespace d3d12
{
	// Global DX12 Constants
	#define DX12_NUM_FRAMES 2
	#define DX12_CB_ALIGNEMENT_SIZE 256

	// Forward declarations
	struct DX12GraphicsDevice;
	struct DX12Window;
	struct DX12CommandQueue;
	struct DX12CommandBuffer;
	struct DX12SwapChain;

	struct DX12Texture;
	struct DX12RenderTexture;
	struct DX12Sampler;
	struct DX12GraphicsBuffer;

	struct DX12DescriptorHeap;
	struct DX12RootSignature;
	struct DX12ComputeShader;
	struct DX12GraphicsPipeline;
	struct DX12Query;

	enum OpaqueType
	{
		// General graphics objects
		GraphicsDeviceT,
		RenderWindowT,
		SwapChainT,
		CommandQueueT,
		CommandBufferT,

		// Shaders objects
		ComputeShaderT,
		GraphicsPipelineT,
		RayTracingShaderT,

		// Syncrhonization
		FenceT,

		// Resources objects
		TextureT,
		RenderTextureT,
		GraphicsBufferT,
		ConstantBufferT,
		SamplerT,
		TopLevelAST,
		BottomLevelAST,

		// Profiling
		ProfilingScopeT,
	};

	struct DX12GraphicsDevice
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::GraphicsDeviceT;
		const OpaqueType opaqueType = OpaqueType::GraphicsDeviceT;
#endif

		// Device instance
		ID3D12Device14* device = nullptr;
		ID3D12DevicePreview* previewDevice = nullptr;

		// Device name
		std::string adapterName = "";

		// Device vendor
		GPUVendor vendor = GPUVendor::Other;

		// Descriptor sizes
		uint32_t descriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

		// Feature support
		bool supportRayTracing = false;
		bool supportWaveMMA = false;
		bool supportDoubleShaderOps = false;
		bool support16bitShaderOps = false;
		bool supportMeshShaders = false;
		bool supportWorkGraph = false;
		bool supportCoopVectors = false;

		// Tracks if the device was created with the debug option
		bool debugDevice = false;

		// Additional stats
		uint64_t allocatedMemory = 0;
		uint32_t allocatedTextures = 0;
		uint32_t allocatedBuffers = 0;
		uint32_t allocatedCS = 0;
		uint32_t allocatedGP = 0;
		uint32_t allocatedSamplers = 0;
	};

	struct DX12Window
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::RenderWindowT;
		const OpaqueType opaqueType = OpaqueType::RenderWindowT;
#endif

		// Actual window
		HWND window = nullptr;
	};

	struct DX12CommandSubQueue
	{
		// DX12 Queue
		ID3D12CommandQueue* queue = nullptr;

		// Priority
		D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

		// Frequency
		uint64_t frequency = UINT64_MAX;

		// Fence used to track the execution
		ID3D12Fence* fence = nullptr;

		// Event to track back the CPU
		HANDLE fenceEvent = nullptr;

		// Fence value
		uint64_t fenceValue = UINT64_MAX;
	};

	struct DX12CommandQueue
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::CommandQueueT;
		const OpaqueType opaqueType = OpaqueType::CommandQueueT;
#endif

		// Graphics device
		DX12GraphicsDevice* deviceI = nullptr;

		// Direct queue
		DX12CommandSubQueue directSubQueue = {};

		// Compute queue
		DX12CommandSubQueue computeSubQueue = {};

		// Copy queue
		DX12CommandSubQueue copySubQueue = {};
	};

	struct DX12Texture
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::TextureT;
		const OpaqueType opaqueType = OpaqueType::TextureT;
#endif

		// Graphics device
		DX12GraphicsDevice* deviceI = nullptr;

		// Actual resource
		ID3D12Resource* resource = nullptr;

		// Current state of the resource
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;

		// Internal properties
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 0;
		uint32_t mipLevels = 0;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		uint32_t alignment = 0;
		TextureType type = TextureType::Tex2D;
		bool isDepth = false;
	};

	struct DX12RenderTexture
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::RenderTextureT;
		const OpaqueType opaqueType = OpaqueType::RenderTextureT;
#endif

		// Actual resource
		DX12Texture texture;

		// Graphics device
		DX12GraphicsDevice* deviceI = nullptr;

		// Descriptor heap where the view is stored
		ID3D12DescriptorHeap* descriptorHeap = nullptr;
		uint32_t heapOffset = UINT32_MAX;
	};

	struct DX12SwapChain
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::SwapChainT;
		const OpaqueType opaqueType = OpaqueType::SwapChainT;
#endif

		// Swap chain
		IDXGISwapChain4* swapChain = nullptr;

		// Tracker of the current back buffer
		uint32_t currentBackBuffer = UINT32_MAX;

		// Descriptor heap where the views for the render targets are stored
		ID3D12DescriptorHeap* descriptorHeap = nullptr;

		// Back Buffers
		DX12RenderTexture backBufferRenderTextures[DX12_NUM_FRAMES] = {};
	};

	struct DX12CommandBuffer
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::CommandBufferT;
		const OpaqueType opaqueType = OpaqueType::CommandBufferT;
#endif

		// Graphics device
		DX12GraphicsDevice* deviceI = nullptr;

		// Allocator for this command buffer
		ID3D12CommandAllocator* commandAllocator_internal[DX12_NUM_FRAMES];

		// Command this for this command buffer
		ID3D12GraphicsCommandListPreview* commandList_internal[DX12_NUM_FRAMES];

		// Frame index for picking the device command primitives
		uint32_t frameIdx = UINT32_MAX;

		// Command buffer type
		D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		// Grab the current command allocator
		inline ID3D12CommandAllocator* cmdAlloc()
		{
			return commandAllocator_internal[frameIdx % DX12_NUM_FRAMES];
		}

		// Grab the current command list
		inline ID3D12GraphicsCommandListPreview* cmdList()
		{
			return commandList_internal[frameIdx % DX12_NUM_FRAMES];
		}
	};

	struct DX12Sampler
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::SamplerT;
		const OpaqueType opaqueType = OpaqueType::SamplerT;
#endif

		// Graphics device
		DX12GraphicsDevice* deviceI = nullptr;

		// Actual resource
		SamplerDescriptor resource = {};
	};

	struct DX12DescriptorHeap
	{
		// Actual heap
		ID3D12DescriptorHeap* descriptorHeap;

		// Type of this heap
		D3D12_DESCRIPTOR_HEAP_TYPE type;

		// GPU Handles for every resource type
		D3D12_GPU_DESCRIPTOR_HANDLE srvGPU;
		D3D12_GPU_DESCRIPTOR_HANDLE uavGPU;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvGPU;
		D3D12_GPU_DESCRIPTOR_HANDLE samplerGPU;

		// CPU Handles for every resource type
		D3D12_CPU_DESCRIPTOR_HANDLE srvCPU;
		D3D12_CPU_DESCRIPTOR_HANDLE uavCPU;
		D3D12_CPU_DESCRIPTOR_HANDLE cbvCPU;
		D3D12_CPU_DESCRIPTOR_HANDLE samplerCPU;
	};

	struct DX12RootSignature
	{
		// Actual root rignature
		ID3D12RootSignature* rootSignature = nullptr;

		// Indices of the different recources
		uint32_t srvIndex = UINT32_MAX;
		uint32_t uavIndex = UINT32_MAX;
		uint32_t cbvIndex = UINT32_MAX;
		uint32_t samplerIndex = UINT32_MAX;
	};

	struct DX12Binding
	{
		uint8_t type;
		uint8_t slot;
	};

	struct DX12ComputeShader
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::ComputeShaderT;
		const OpaqueType opaqueType = OpaqueType::ComputeShaderT;
#endif

		// General
		DX12GraphicsDevice* device = nullptr;
		DX12RootSignature* rootSignature = nullptr;
		ID3D12PipelineState* pipelineStateObject = nullptr;

		// Shader code
		IDxcBlob* shaderBlob = nullptr;

		// Number of resources
		uint32_t cbvCount = 0;
		uint32_t srvCount = 0;
		uint32_t uavCount = 0;
		uint32_t samplerCount = 0;

		// Reflection data
		std::map<std::string, DX12Binding> bindings;

		// Set of descriptor heaps that can be used for this compute shader
		uint32_t cmdBatchIndex = 0;
		uint32_t nextUsableHeap = 0;
		std::vector<DX12DescriptorHeap> CSUHeaps;
		std::vector<DX12DescriptorHeap> samplerHeaps;

		// Command signature for indirect dispatch
		ID3D12CommandSignature* commandSignature = nullptr;

		// Barriers to enqueue
		std::vector<D3D12_RESOURCE_BARRIER> barriersData;
	};

	struct DX12GraphicsPipeline
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::GraphicsPipelineT;
		const OpaqueType opaqueType = OpaqueType::GraphicsPipelineT;
#endif

		// General
		DX12GraphicsDevice* device = nullptr;
		DX12RootSignature* rootSignature = nullptr;
		ID3D12PipelineState* pipelineStateObject = nullptr;

		// Vertex pipeline
		IDxcBlob* vertBlob = nullptr;
		IDxcBlob* hullblob = nullptr;
		IDxcBlob* domainBlob = nullptr;
		IDxcBlob* geometryBlob = nullptr;

		// Fragment code
		IDxcBlob* fragBlob = nullptr;

		// Number of resources
		uint32_t srvCount = 0;
		uint32_t uavCount = 0;
		uint32_t cbvCount = 0;
		uint32_t samplerCount = 0;

		// Reflection data
		std::map<std::string, DX12Binding> bindings;

		// Set of descriptor heaps that can be used for this compute shader
		uint32_t cmdBatchIndex = 0;
		uint32_t nextUsableHeap = 0;
		std::vector<DX12DescriptorHeap> CSUHeaps;
		std::vector<DX12DescriptorHeap> samplerHeaps;

		// Stencil ref
		uint8_t stencilRef = 0;

		// Command signature for indirect dispatch
		ID3D12CommandSignature* commandSignature = nullptr;

		// Barriers to enqueue
		std::vector<D3D12_RESOURCE_BARRIER> barriersData;
	};

	struct DX12GraphicsBuffer
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::GraphicsBufferT;
		const OpaqueType opaqueType = OpaqueType::GraphicsBufferT;
#endif

		// General
		DX12GraphicsDevice* device = nullptr;

		// Actual resource
		ID3D12Resource* resource = nullptr;

		// Current state
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;

		// Properties of the buffer
		uint64_t bufferSize = 0;
		uint32_t elementSize = 0;
		GraphicsBufferType heapType = GraphicsBufferType::Default;
	};

	struct DX12Query
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::ProfilingScopeT;
		const OpaqueType opaqueType = OpaqueType::ProfilingScopeT;
#endif

		// Heap for the query
		ID3D12QueryHeap* heap = nullptr;

		// Resource that holds the result of the query
		ID3D12Resource* result = nullptr;

		// State pf the resource
		D3D12_RESOURCE_STATES state;
	};

	struct DX12ConstantBuffer
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::ConstantBufferT;
		const OpaqueType opaqueType = OpaqueType::ConstantBufferT;
#endif

		// Type of the constant buffer
		ConstantBufferType type = ConstantBufferType::Runtime;

		// List of sub-reousrces of the constant buffer
		DX12GraphicsBuffer* intermediateBuffers_internal[DX12_NUM_FRAMES] = { nullptr, nullptr };
		DX12GraphicsBuffer* mainBuffer = nullptr;

		// Actual element size after alignement
		uint32_t elementSize = 0;

		// Which instance should we be using?
		uint32_t instanceIdx = 0;

		inline DX12GraphicsBuffer* intermediateBuffer()
		{
			return intermediateBuffers_internal[instanceIdx];
		}
	};

	struct DX12BLAS
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::BottomLevelAST;
		const OpaqueType opaqueType = OpaqueType::BottomLevelAST;
#endif

		// BLAS buffer
		DX12GraphicsBuffer* data = nullptr;

		// Scratch buffer
		DX12GraphicsBuffer* scratchBuffer = nullptr;

		// Build data
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};

		// Buffers used
		DX12GraphicsBuffer* vertexBuffer = nullptr;
		DX12GraphicsBuffer* indexBuffer = nullptr;
	};

	struct DX12TLAS
	{
#if defined(_DEBUG)
		// Type checking
		static const OpaqueType s_opaqueType = OpaqueType::TopLevelAST;
		const OpaqueType opaqueType = OpaqueType::TopLevelAST;
#endif

		// TLAS buffer
		DX12GraphicsBuffer* data = nullptr;

		// Scratch buffer
		DX12GraphicsBuffer* scratchBuffer = nullptr;

		// Build data
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		DX12GraphicsBuffer* instanceBufer = nullptr;
		std::vector< D3D12_RAYTRACING_INSTANCE_DESC> instanceArray;
	};

	template<typename T>
	T* safe_convert(uint64_t ptr)
	{
		T* t = (T*)ptr;
#if defined(_DEBUG)
		assert_msg(t->opaqueType == T::s_opaqueType, "Failed to validate type");
#endif
		return t;
	}
}
