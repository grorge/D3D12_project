#include "DescriptorHeap.h"



DescriptorHeap::DescriptorHeap()
{
	mp_descriptorHeap = nullptr;
}

DescriptorHeap::~DescriptorHeap()
{
	Destroy();
}

void DescriptorHeap::Initialize(
	ID3D12Device4 * pDevice,
	const D3D12_DESCRIPTOR_HEAP_TYPE type,
	const UINT count)
{
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = count;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;
		
		pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mp_descriptorHeap));
	}
}

void DescriptorHeap::Destroy()
{
	if (mp_descriptorHeap)
	{
		mp_descriptorHeap->Release();
		mp_descriptorHeap = nullptr;
	}
}
