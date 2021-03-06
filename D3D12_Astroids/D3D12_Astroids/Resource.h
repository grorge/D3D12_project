#pragma once

#include "D3DHeader.h"

class Resource 
{
public:
	ID3D12Resource* mp_resource;
	D3D12_RESOURCE_STATES m_currentState;

	virtual void Initialize(
		ID3D12Device * pDevice,
		const UINT byteWidth,
		const D3D12_HEAP_FLAGS heapFlag,
		const D3D12_RESOURCE_STATES state,
		const D3D12_RESOURCE_FLAGS resourceFlag) = 0;

	void Destroy() { SafeRelease(&mp_resource); };

protected:

	UINT m_byteWidth;
};
