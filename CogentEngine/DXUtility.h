#pragma once

#include <wrl.h>
#include <assert.h>
#include <d3d12.h>
//#include "ResourceManager.h"
#include <string>

#define SafeRelease(comObj) if(comObj) comObj->Release();

constexpr uint64_t AlignmentSize = 256;

std::wstring ToWString(const std::string& str);

//class GPUConstantBuffer
//{
//public:
//	GPUConstantBuffer() {};
//	void Initialize(ResourceManager* rm, const uint64_t cbSize, const uint32 count)
//	{
//		constBufferSize = (cbSize + AlignmentSize - 1) & ~(AlignmentSize - 1);
//		bufferSize = constBufferSize * count;
//		resourceID = rm->CreateResource(
//			CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
//			nullptr,
//			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_FLAG_NONE,
//			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)
//		);
//
//		resource = rm->GetResource(resourceID);
//		CD3DX12_RANGE readRange(0, 0);
//		resource->Map(0, &readRange, reinterpret_cast<void**>(&vAddressBegin));
//	}
//
//	void Initialize(ResourceManager* rm, const uint64_t bufferSize)
//	{
//		constBufferSize = AlignmentSize;
//		this->bufferSize = bufferSize;
//		resourceID = rm->CreateResource(
//			CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
//			nullptr,
//			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_FLAG_NONE,
//			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)
//		);
//
//		resource = rm->GetResource(resourceID);
//		CD3DX12_RANGE readRange(0, 0);
//		resource->Map(0, &readRange, reinterpret_cast<void**>(&vAddressBegin));
//	}
//
//	void CopyData(void* data, uint64_t size, uint32 index) const
//	{
//		char* ptr = reinterpret_cast<char*>(vAddressBegin) + (size_t)constBufferSize * index;
//		memcpy(ptr, data, size);
//	}
//
//	void CopyData(void* data, uint64_t size, uint64_t offset) const
//	{
//		char* ptr = reinterpret_cast<char*>(vAddressBegin) + offset;
//		memcpy(ptr, data, size);
//	}
//
//	void CopyData(void* data, uint64_t size) const
//	{
//		memcpy(vAddressBegin, data, size);
//	}
//
//	D3D12_GPU_VIRTUAL_ADDRESS GetAddress(uint32 index = 0) const
//	{
//		return resource->GetGPUVirtualAddress() + (size_t)constBufferSize * index;
//	}
//
//	~GPUConstantBuffer() {}
//
//private:
//	ID3D12Resource* resource;
//	//ResourceID resourceID;
//	uint64_t constBufferSize;
//	uint64_t bufferSize;
//	char* vAddressBegin;
//};

class DescriptorHeap
{
public:
	DescriptorHeap() { memset(this, 0, sizeof(*this)); }

	HRESULT Create(
		ID3D12Device* pDevice,
		D3D12_DESCRIPTOR_HEAP_TYPE Type,
		UINT NumDescriptors,
		bool bShaderVisible = false)
	{
		HeapDesc.Type = Type;
		HeapDesc.NumDescriptors = NumDescriptors;
		HeapDesc.Flags = (bShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : (D3D12_DESCRIPTOR_HEAP_FLAGS)0);

		HRESULT hr = pDevice->CreateDescriptorHeap(&HeapDesc,
			__uuidof(ID3D12DescriptorHeap),
			(void**)& pDescriptorHeap);
		if (FAILED(hr)) return hr;
		pDescriptorHeap->SetName(ToWString(std::to_string(NumDescriptors) + " Heap").c_str());
		hCPUHeapStart = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (bShaderVisible)
		{
			hGPUHeapStart = pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}
		else
		{
			hGPUHeapStart.ptr = 0;
		}
		HandleIncrementSize = pDevice->GetDescriptorHandleIncrementSize(HeapDesc.Type);
		return hr;
	}
	operator ID3D12DescriptorHeap* () { return pDescriptorHeap.Get(); }

	//size_t MakeOffsetted(size_t ptr, UINT index)
	//{
	//	size_t offsetted;
	//	offsetted = ptr + static_cast<size_t>(index * HandleIncrementSize);
	//	return offsetted;
	//}

	uint64_t MakeOffsetted(uint64_t ptr, UINT index) const
	{
		uint64_t offsetted;
		offsetted = ptr + static_cast<uint64_t>(index * HandleIncrementSize);
		return offsetted;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU(UINT index) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = MakeOffsetted(hCPUHeapStart.ptr, index);
		return handle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU(UINT index) const
	{
		assert(HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		D3D12_GPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = MakeOffsetted(hGPUHeapStart.ptr, index);
		return handle;
	}
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE hCPUHeapStart;
	D3D12_GPU_DESCRIPTOR_HANDLE hGPUHeapStart;
	UINT HandleIncrementSize;
};