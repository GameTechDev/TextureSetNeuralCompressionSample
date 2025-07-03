/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Internal includes
#include "dx12/dx12_backend.h"
#include "dx12/dx12_containers.h"
#include "dx12/dx12_helpers.h"
#include "tools/security.h"
#include "tools/string_utilities.h"

// System includes
#include <algorithm>
#include <initguid.h>
#include "dxgidebug.h"
#include <inttypes.h>

// Export if we are running in experimental
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = DX12_SDK_VERSION; }
extern "C" { __declspec(dllexport) extern const char8_t* D3D12SDKPath = u8".\\D3D12\\"; }

// Tracking if the debug layer was enabled
static bool g_debugLayerEnabled = false;

namespace d3d12
{
    namespace device
    {
        IDXGIAdapter1* GetLargestAdapter()
        {
            // Create the DXGI factory
            IDXGIFactory4* dxgiFactory;
            UINT createFactoryFlags = 0;
            assert_msg(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)) == S_OK, "DXGI Factory 2 failed.");

            // The final adapter that we will be using
            IDXGIAdapter1* targetAdapter = nullptr;

            // Loop through all the available dapters
            IDXGIAdapter1* dxgiAdapter1;
            SIZE_T maxDedicatedVideoMemory = 0;

            // Loop through the adapters
            for (uint32_t adapterIdx = 0; dxgiFactory->EnumAdapters1(adapterIdx, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++adapterIdx)
            {
                // Grab the descriptor of this adapter
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // First is it usable?
                // bool usableGPU = (dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1, D3D_FEATURE_LEVEL_12_2, __uuidof(ID3D12Device), nullptr));

                // Is it bigger?
                if (dxgiAdapterDesc1.DedicatedVideoMemory >= maxDedicatedVideoMemory)
                {
                    // New size
                    maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;

                    // Keep track of the adapter
                    targetAdapter = dxgiAdapter1;
                }
            }

            // Release the factory
            dxgiFactory->Release();

            // Return the adapter
            return targetAdapter;
        }

        IDXGIAdapter1* GetAdapterGPUVendor(GPUVendor vendor)
        {
            // Get the vendor ID
            uint32_t vendorID = vendor_to_vendor_id(vendor);

            // Create the DXGI factory
            IDXGIFactory4* dxgiFactory;
            UINT createFactoryFlags = 0;
            assert_msg(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)) == S_OK, "DXGI Factory 2 failed.");

            // The final adapter that we will be using
            IDXGIAdapter1* targetAdapter = nullptr;

            // Loop through all the available dapters
            IDXGIAdapter1* dxgiAdapter1;

            // Loop through the adapters
            for (uint32_t adapterIdx = 0; dxgiFactory->EnumAdapters1(adapterIdx, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++adapterIdx)
            {
                // Grab the descriptor of this adapter
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // First is it usable?
                bool usableGPU = (dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1, D3D_FEATURE_LEVEL_12_2, __uuidof(ID3D12Device), nullptr));

                // Is it the right vendor?
                if (usableGPU && dxgiAdapterDesc1.VendorId == vendorID)
                {
                    targetAdapter = dxgiAdapter1;
                    break;
                }
            }

            // Release the factory
            dxgiFactory->Release();

            // Return the adapter
            return targetAdapter;
        }

        IDXGIAdapter1* GetAdapterbyID(uint32_t adapterID)
        {
            // Create the DXGI factory
            IDXGIFactory4* dxgiFactory;
            UINT createFactoryFlags = 0;
            assert_msg(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)) == S_OK, "DXGI Factory 2 failed.");

            // The final adapter that we will be using
            IDXGIAdapter1* targetAdapter = nullptr;

            // Loop through all the available dapters
            IDXGIAdapter1* dxgiAdapter1;

            // Loop through the adapters
            for (uint32_t adapterIdx = 0; dxgiFactory->EnumAdapters1(adapterIdx, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++adapterIdx)
            {
                // Grab the descriptor of this adapter
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // First is it usable?
                bool usableGPU = (dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr));

                // Is it the right vendor?
                if (usableGPU && adapterIdx == adapterID)
                    targetAdapter = dxgiAdapter1;
                else
                    dxgiAdapter1->Release();
            }

            // Release the factory
            dxgiFactory->Release();

            // Return the adapter
            return targetAdapter;
        }

        // pre-creation functions
        void enable_experimental_features()
        {
            UUID Features[2] = { D3D12ExperimentalShaderModels, D3D12CooperativeVectorExperiment };
            assert_msg(D3D12EnableExperimentalFeatures(_countof(Features), Features, nullptr, nullptr) == S_OK, "Failed to enable experimental features.");
        }

        void enable_debug_layer()
        {
            ID3D12Debug6* debugLayer = nullptr;
            assert_msg(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)) == S_OK, "Debug layer failed to initialize");
            debugLayer->EnableDebugLayer();
            g_debugLayerEnabled = true;
            debugLayer->Release();
        }

        GraphicsDevice create_graphics_device(DevicePickStrategy pickStrategy, uint32_t id)
        {
            // Create the graphics device internal structure
            DX12GraphicsDevice* dx12_device = new DX12GraphicsDevice();
            dx12_device->debugDevice = g_debugLayerEnabled;

            // Grab the right adapter based on the requested strategy
            IDXGIAdapter1* adapter = nullptr;
            if (pickStrategy ==  DevicePickStrategy::VRAMSize)
                adapter = GetLargestAdapter();
            else if (pickStrategy == DevicePickStrategy::VendorID)
                adapter = GetAdapterGPUVendor((GPUVendor)id);
            else if (pickStrategy == DevicePickStrategy::AdapterID)
                adapter = GetAdapterbyID(id);

            // Grab the descriptor of this adapter
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            adapter->GetDesc1(&dxgiAdapterDesc1);
            dx12_device->adapterName = convert_to_regular(dxgiAdapterDesc1.Description);
            wprintf(L"Picked GPU: %s\n", dxgiAdapterDesc1.Description);
            // wprintf(L"GPU Memory: %" PRIu64 L"\n", dxgiAdapterDesc1.DedicatedVideoMemory);

            // Create the graphics device and ensure it's been succesfully created
            HRESULT result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&dx12_device->device));
            assert_msg(result == S_OK, "D3D12 Device creation failed.");

            // Do not forget to release the adapter
            adapter->Release();

            // Keep track of the structures and data
            for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
                dx12_device->descriptorSize[i] = dx12_device->device->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)i);

            // Get the vendor
            dx12_device->vendor = vendor_id_to_vendor(dxgiAdapterDesc1.VendorId);

            // Double support
            D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)) == S_OK, "Failed to query option.");
            dx12_device->supportDoubleShaderOps = options.DoublePrecisionFloatShaderOps;

            // Shader 16-bits
            D3D12_FEATURE_DATA_D3D12_OPTIONS4 options4 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &options4, sizeof(options4)) == S_OK, "Failed to query option4.");
            dx12_device->support16bitShaderOps = options4.Native16BitShaderOpsSupported;

            // Ray tracing
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)) == S_OK, "Failed to query option5.");
            dx12_device->supportRayTracing = options5.RaytracingTier > D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

            // Wave MMA
            D3D12_FEATURE_DATA_D3D12_OPTIONS9 options9 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, &options9, sizeof(options9)) == S_OK, "Failed to query option9.");
            dx12_device->supportWaveMMA = options9.WaveMMATier != D3D12_WAVE_MMA_TIER_NOT_SUPPORTED;

            // Mesh shaders
            D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)) == S_OK, "Failed to query option7.");
            dx12_device->supportMeshShaders = options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;

            // Work graph
            D3D12_FEATURE_DATA_D3D12_OPTIONS21 options21 = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options21, sizeof(options21)) == S_OK, "Failed to query option21.");
            dx12_device->supportWorkGraph = options21.WorkGraphsTier != D3D12_WORK_GRAPHS_TIER_1_0;

            // Coop vectors
            D3D12_FEATURE_DATA_D3D12_OPTIONS_EXPERIMENTAL expFeatures = {};
            assert_msg(dx12_device->device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS_EXPERIMENTAL, &expFeatures, sizeof(expFeatures)) == S_OK, "Failed to query experimental.");
            dx12_device->supportCoopVectors = expFeatures.CooperativeVectorTier != D3D12_COOPERATIVE_VECTOR_TIER_NOT_SUPPORTED;

            // Query preview device if possible
            if (dx12_device->supportCoopVectors)
                assert_msg(dx12_device->device->QueryInterface(IID_PPV_ARGS(&dx12_device->previewDevice)) == S_OK, "Failed to query preview device.");

            return (GraphicsDevice)dx12_device;
        }

        void destroy_graphics_device(GraphicsDevice graphicsDevice)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)graphicsDevice;

            // Sanity check
            assert_msg(dx12_device->allocatedTextures == 0
                && dx12_device->allocatedBuffers == 0
                && dx12_device->allocatedSamplers == 0
                && dx12_device->allocatedCS == 0
                && dx12_device->allocatedGP == 0, "Graphics Device has still active resources");

            // Release the devfice
            dx12_device->device->Release();

            // Destroty the internal structure
            delete dx12_device;
        }

        void set_stable_power_state(GraphicsDevice device, bool state)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            dx12_device->device->SetStablePowerState(state);
        }

        GPUVendor get_gpu_vendor(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->vendor;
        }

        const char* get_device_name(GraphicsDevice device)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            return dx12_device->adapterName.c_str();
        }

        bool feature_support(GraphicsDevice device, GPUFeature feature)
        {
            DX12GraphicsDevice* dx12_device = (DX12GraphicsDevice*)device;
            switch (feature)
            {
                case GPUFeature::RayTracing:
                    return dx12_device->supportRayTracing;
                case GPUFeature::DoubleOps:
                    return dx12_device->supportDoubleShaderOps;
                case GPUFeature::HalfOps:
                    return dx12_device->support16bitShaderOps;
                case GPUFeature::MeshShader:
                    return dx12_device->supportMeshShaders;
                case GPUFeature::CoopMatrix:
                    return false;
                case GPUFeature::CoopVector:
                    return dx12_device->supportCoopVectors;
            }
            return false;
        }

        CoopMatTier coop_mat_tier(GraphicsDevice)
        {
            return CoopMatTier::Other;
        }
    }
}
