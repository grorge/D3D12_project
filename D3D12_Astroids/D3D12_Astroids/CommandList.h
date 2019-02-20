#pragma once

#include "D3DHeader.h"

class CommandList
{
public:
	CommandList();
	~CommandList();

	void Initialize(
		ID3D12Device4 * pDevice,
		ID3D12CommandAllocator* pAllocator,
		const D3D12_COMMAND_LIST_TYPE type);

	void Destroy();

	ID3D12GraphicsCommandList3* mp_cmdList;

private:
};