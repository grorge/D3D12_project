#pragma once

#include "D3DHeader.h"

class Resource 
{
public:
	virtual void Initialize(
		ID3D12Device* pDevice,
		const UINT byteWidth,
		const D3D12_HEAP_FLAGS flag,
		const DXGI_FORMAT format) = 0;

	ID3D12Resource* mp_resource;
	D3D12_RESOURCE_STATES m_currentState;



	void Destroy() 
	{
		if (mp_resource)
		{
			mp_resource->Release();
			mp_resource = nullptr;
		}
	};

protected:

	UINT m_byteWidth;
};
