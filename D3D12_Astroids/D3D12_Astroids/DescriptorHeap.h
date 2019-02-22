#pragma once

#include "D3DHeader.h"

class DescriptorHeap
{
public:
	DescriptorHeap();
	~DescriptorHeap();

	void Initialize(
		ID3D12Device4 * pDevice,
		const D3D12_DESCRIPTOR_HEAP_TYPE type,
		const UINT count);

	void Destroy();

	ID3D12DescriptorHeap* mp_descriptorHeap;
private:

};