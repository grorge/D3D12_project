#include "UAVBuffer.h"

UAVBuffer::UAVBuffer()
{
	m_writeAccess = false;
	m_readAccess = false;
}

UAVBuffer::~UAVBuffer()
{
}

void UAVBuffer::Initialize(
	ID3D12Device* pDevice,
	const UINT byteWidth,
	const bool cpuWriteAccess,
	const bool cpuReadAccess)
{
	m_resource.Initialize(
		pDevice,
		byteWidth,
		D3D12_HEAP_FLAG_NONE,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	if (cpuWriteAccess)
	{
		m_writeAccess = cpuWriteAccess;
		m_upload.Initialize(
			pDevice,
			byteWidth,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_FLAG_NONE);
	}

	if (cpuReadAccess)
	{
		m_readAccess = cpuReadAccess;
		m_download.Initialize(
			pDevice,
			byteWidth,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_FLAG_NONE);
	}
}

ID3D12Resource * UAVBuffer::operator()()
{
	return m_resource.mp_resource;
}

bool UAVBuffer::UploadData(void * pData, ID3D12GraphicsCommandList* pCmdList, int size)
{
	if (!m_writeAccess) return false;

	m_upload.SetData(pData, size);

	pCmdList->CopyResource(
		m_resource.mp_resource, 
		m_upload.mp_resource);

	return true;
}

bool UAVBuffer::DownloadData(ID3D12GraphicsCommandList* pCmdList)
{
	if (!m_readAccess) return false;

	pCmdList->CopyResource(
		m_download.mp_resource, 
		m_resource.mp_resource);

	return true;
}

void * UAVBuffer::GetData()
{
	return m_download.GetData();
}
