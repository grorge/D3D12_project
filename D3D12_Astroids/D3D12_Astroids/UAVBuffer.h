#pragma once

#include "DefaultResource.h"
#include "ReadbackResource.h"
#include "UploadResource.h"

class UAVBuffer
{
public:
	UAVBuffer();
	~UAVBuffer();

	void Initialize(
		ID3D12Device* pDevice,
		const UINT byteWidth,
		const bool cpuWriteAccess, 
		const bool cpuReadAccess);

	ID3D12Resource* operator()();

	bool UploadData(void* pData, ID3D12GraphicsCommandList* pCmdList, int size);
	bool DownloadData(ID3D12GraphicsCommandList* pCmdList);
	void* GetData();

private:
	bool m_writeAccess, m_readAccess;

	DefaultResource		m_resource;
	UploadResource		m_upload;
	ReadbackResource	m_download;
};