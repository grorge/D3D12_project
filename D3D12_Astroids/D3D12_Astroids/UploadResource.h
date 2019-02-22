#pragma once

#include "Resource.h"

class UploadResource : public Resource
{
public:
	UploadResource();
	~UploadResource();

	void Initialize(
		ID3D12Device * pDevice,
		const UINT byteWidth,
		const D3D12_HEAP_FLAGS heapFlag,
		const D3D12_RESOURCE_STATES state,
		const D3D12_RESOURCE_FLAGS resourceFlag) override;

	void SetData(const void* data);

private:
};