#include "ReadbackResource.h"



ReadbackResource::ReadbackResource()
{
}

ReadbackResource::~ReadbackResource()
{
	Destroy();
}

void ReadbackResource::Initialize(
	ID3D12Device * pDevice, 
	const UINT byteWidth, 
	const D3D12_HEAP_FLAGS heapFlag, 
	const D3D12_RESOURCE_STATES state, 
	const D3D12_RESOURCE_FLAGS resourceFlag)
{
	m_byteWidth = byteWidth;
	m_currentState = state;

	{
		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = D3D12_HEAP_TYPE_READBACK;
		properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		properties.CreationNodeMask = 0;
		properties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

		/*When set to 0 runtime will use defualt alignment (64KB for buffers)*/
		desc.Alignment = 0; // D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		desc.Width = m_byteWidth;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc = { 1, 0 };
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // Swizzle Layout better?
		desc.Flags = resourceFlag;

		pDevice->CreateCommittedResource(
			&properties,
			heapFlag,
			&desc,
			m_currentState,
			NULL,
			IID_PPV_ARGS(&mp_resource));
	}
}

void * ReadbackResource::GetData()
{
	void* dataBegin = nullptr;

	D3D12_RANGE read = { 0, m_byteWidth };
	D3D12_RANGE write = { 0, 0 };
	mp_resource->Map(0, &read, &dataBegin);
	mp_resource->Unmap(0, &write);

	return dataBegin;
}
