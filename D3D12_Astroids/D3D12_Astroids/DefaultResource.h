#pragma once

#include "Resource.h"

class DefaultResource : public Resource
{
public:
	DefaultResource();
	~DefaultResource();

	void Initialize(
		ID3D12Device * pDevice,
		const UINT byteWidth,
		const D3D12_HEAP_FLAGS flag,
		const D3D12_RESOURCE_STATES state) override;

private:

};