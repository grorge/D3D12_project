#pragma once

#include "D3DHeader.h"

class CommandAllocator
{
public:
	CommandAllocator();
	~CommandAllocator();

	void Initialize(
		ID3D12Device4 * pDevice,
		const D3D12_COMMAND_LIST_TYPE type);

	void Destroy();

	ID3D12CommandAllocator* mp_cmdAllocator;

private:
};