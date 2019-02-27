#pragma once

#include "Resource.h"

class ReadbackResource : public Resource
{
public:
	ReadbackResource();
	~ReadbackResource();

	void Initialize(
		ID3D12Device * pDevice,
		const UINT byteWidth,
		const D3D12_HEAP_FLAGS heapFlag,
		const D3D12_RESOURCE_STATES state,
		const D3D12_RESOURCE_FLAGS resourceFlag) override;

	void* GetData();

private:

};