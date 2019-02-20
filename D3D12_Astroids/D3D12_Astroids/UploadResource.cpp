#include "UploadResource.h"



UploadResource::UploadResource()
{
	m_byteWidth = 0;
	mp_resource = nullptr;
	m_currentState = D3D12_RESOURCE_STATE_COMMON;
}

UploadResource::~UploadResource()
{
	Destroy();
}

void UploadResource::Initialize(
	ID3D12Device * pDevice,
	const UINT byteWidth,
	const D3D12_HEAP_FLAGS flag,
	const DXGI_FORMAT format)
{
	m_byteWidth = byteWidth;
	m_currentState = D3D12_RESOURCE_STATE_GENERIC_READ;

	{

		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = D3D12_HEAP_TYPE_UPLOAD;
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
		desc.Format = format;
		desc.SampleDesc = { 1, 0 };
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // Swizzle Layout better?
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		pDevice->CreateCommittedResource(
			&properties,
			flag,
			&desc,
			m_currentState,
			NULL,
			IID_PPV_ARGS(&mp_resource));
	}
}

void UploadResource::SetData(const void * data)
{
	void* dataBegin = nullptr;
	mp_resource->Map(0, nullptr, &dataBegin);
	memcpy(dataBegin, data, m_byteWidth);
	mp_resource->Unmap(0, nullptr);
}
