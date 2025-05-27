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
#include "tools/string_utilities.h"
#include "tools/security.h"

// System includes
#include <unordered_set>
#include <fstream>
#include <dxcapi.h>

namespace d3d12
{
	IDxcBlob* LoadDXILShader(const std::wstring& filename)
	{
		IDxcUtils* dxcUtils;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));

		IDxcBlobEncoding* blobEncoding;
		dxcUtils->LoadFile(filename.c_str(), nullptr, &blobEncoding);

		dxcUtils->Release();
		return blobEncoding;
	}
	namespace compute_shader
	{
		ComputeShader create_compute_shader(GraphicsDevice graphicsDevice, const ComputeShaderDescriptor& csd, bool experimental)
		{
			// Convert the strings to wide
			const std::wstring& filename = convert_to_wide(csd.filename.c_str(), (uint32_t)csd.filename.size());
			const std::wstring& kernelName = convert_to_wide(csd.kernelname.c_str(), (uint32_t)csd.kernelname.size());

			// Convert the device
			DX12GraphicsDevice* deviceI = (DX12GraphicsDevice*)graphicsDevice;
			ID3D12Device2* device = deviceI->device;

			// Create our internal structure
			DX12ComputeShader* cS = new DX12ComputeShader();

			D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc = {};

			// Create the library and the compiler
			IDxcLibrary* library;
			DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

			// Load the file into a blob
			uint32_t code_page = CP_UTF8;
			IDxcBlobEncoding* source_blob;
			assert_msg(library->CreateBlobFromFile(filename.c_str(), &code_page, &source_blob) == S_OK, "Failed to load the shader code.");

			// Compilation arguments
			std::vector<std::wstring> includeDirs;
			for (int includeDirIdx = 0; includeDirIdx < csd.includeDirectories.size(); ++includeDirIdx)
			{
				std::wstring includeArg = L"-I ";
				includeArg += convert_to_wide(csd.includeDirectories[includeDirIdx]);
				includeDirs.push_back(includeArg.c_str());
			}

			std::vector<LPCWSTR> arguments;
			arguments.push_back(L"-HV 2021");
			if (!csd.debugFlag)
			{
				arguments.push_back(L"-O3");
			}
			else
			{
				arguments.push_back(L"-Od");
				arguments.push_back(L"-Fd");
				arguments.push_back(L"-Qembed_debug");
				arguments.push_back(L"-Zi");
			}

			// Enable 16bit types if available
			if (deviceI->support16bitShaderOps)
				arguments.push_back(L"-enable-16bit-types");

			// Handle the defines
			std::vector<DxcDefine> definesArray(csd.defines.size());
			std::vector<std::wstring> definesWSTR(csd.defines.size());
			for (int defIdx = 0; defIdx < csd.defines.size(); ++defIdx)
			{
				// Convert and keep it for memory management issues
				definesWSTR[defIdx] = convert_to_wide(csd.defines[defIdx]);

				// Add the define
				DxcDefine def;
				def.Name = definesWSTR[defIdx].c_str();
				def.Value = L"";
				definesArray[defIdx] = def;
			}

			// Mark the double unsupported if they are
			if (!deviceI->supportDoubleShaderOps)
			{
				DxcDefine def;
				def.Name = L"FP64_UNSUPPORTED";
				def.Value = L"1";
				definesArray.push_back(def);
			}

			if (deviceI->vendor == GPUVendor::Intel)
			{
				DxcDefine def;
				def.Name = L"UNSUPPORTED_FIRST_BIT_HIGH";
				def.Value = L"1";
				definesArray.push_back(def);
			}

			// Handle the include dirs
			for (int includeDirIdx = 0; includeDirIdx < csd.includeDirectories.size(); ++includeDirIdx)
				arguments.push_back(includeDirs[includeDirIdx].c_str());

			// Create the compiler
			IDxcCompiler* compiler;
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

			// Create an include handler
			IDxcIncludeHandler* includeHandler;
			library->CreateIncludeHandler(&includeHandler);

			// Compile the shader
			IDxcOperationResult* result;
			HRESULT hr = compiler->Compile(source_blob, filename.c_str(), kernelName.c_str(), experimental ? L"cs_6_9" : L"cs_6_6", arguments.data(), (uint32_t)arguments.size(), definesArray.data(), (uint32_t)definesArray.size(), includeHandler, &result);

			if (SUCCEEDED(hr))
				result->GetStatus(&hr);
			bool compile_succeed = SUCCEEDED(hr);

			// If the compilation failed, print the error
			IDxcBlobEncoding* error_blob;
			if (SUCCEEDED(result->GetErrorBuffer(&error_blob)) && error_blob)
			{
				// Log the compilation message
				if (error_blob->GetBufferSize() != 0)
					printf("[SHADER COMPILATION] %s, %s\n", csd.kernelname.c_str(), (const char*)error_blob->GetBufferPointer());

				// Release the error blob
				error_blob->Release();
			}

			// Release the library
			library->Release();

			// If succeeded, grab the right pointer
			IDxcBlob* shader_blob = 0;
			if (compile_succeed)
				result->GetResult(&shader_blob);

			// Release all the intermediate resources
			result->Release();
			source_blob->Release();
			compiler->Release();

			// If we were not able to compile, leave.
			if (shader_blob == nullptr)
				return 0;

			// Keep track of the blob
			cS->shaderBlob = shader_blob;

			// Create the pipeline state object for the shader
			pso_desc.CS.BytecodeLength = shader_blob->GetBufferSize();
			pso_desc.CS.pShaderBytecode = shader_blob->GetBufferPointer();
			
			// Do the reflection
			uint32_t cbvCount = 0, srvCount = 0, uavCount = 0, samplerCount = 0;
			query_bindings(cS->shaderBlob, cbvCount, srvCount, uavCount, samplerCount, cS->bindings);

			// Build the root signature
			cS->rootSignature = create_root_signature(deviceI, srvCount, uavCount, cbvCount, samplerCount);
			pso_desc.pRootSignature = cS->rootSignature->rootSignature;

			// Create the pipeline state object
			ID3D12PipelineState* pso;
			hr = device->CreateComputePipelineState(&pso_desc, IID_PPV_ARGS(&pso));
			assert_msg(hr == S_OK, "Failed to create pipeline state object.");

			// Fill the compute shader structure
			cS->device = deviceI;
			cS->pipelineStateObject = pso;	
			cS->cbvCount = cbvCount;
			cS->srvCount = srvCount;
			cS->uavCount = uavCount;
			cS->samplerCount = samplerCount;

			// Create the descriptor heap for this compute shader
			cS->CSUHeaps.push_back(create_descriptor_heap_suc(deviceI, srvCount, uavCount, cbvCount));
			cS->samplerHeaps.push_back(create_descriptor_heap_sampler(deviceI, std::max(samplerCount, 1u)));
			cS->cmdBatchIndex = UINT32_MAX;
			cS->nextUsableHeap = 0;

			// Create the command signature and append it
			D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1];
			argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
			D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
			commandSignatureDesc.pArgumentDescs = argumentDescs;
			commandSignatureDesc.NumArgumentDescs = 1;
			commandSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
			assert(deviceI->device->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&cS->commandSignature)) == S_OK);

			// Resource counting
			deviceI->allocatedCS++;

			// Convert to the opaque structure
			return (ComputeShader)cS;
		}

		void destroy_compute_shader(ComputeShader computeShader)
		{
			// Grab the internal structure
			DX12ComputeShader* dx12_computeShader = (DX12ComputeShader*)computeShader;

			// Destroy all the descriptor heaps
			uint32_t numDescriptorHeaps = (uint32_t)dx12_computeShader->CSUHeaps.size();
			for (uint32_t heapIdx = 0; heapIdx < numDescriptorHeaps; ++heapIdx)
			{
				destroy_descriptor_heap(dx12_computeShader->CSUHeaps[heapIdx]);
				destroy_descriptor_heap(dx12_computeShader->samplerHeaps[heapIdx]);
			}

			dx12_computeShader->commandSignature->Release();
			dx12_computeShader->shaderBlob->Release();
			dx12_computeShader->pipelineStateObject->Release();
			destroy_root_signature(dx12_computeShader->rootSignature);

			// Resource counting
			dx12_computeShader->device->allocatedCS--;

			// Destroy the internal structure
			delete dx12_computeShader;
		}
	}
}
