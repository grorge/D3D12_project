#pragma once

#include "D3DHeader.h"

class CommandQueue
{
public:
	CommandQueue();
	~CommandQueue();

	void Initialize(
		ID3D12Device * pDevice,
		const D3D12_COMMAND_LIST_TYPE type,
		const D3D12_COMMAND_QUEUE_PRIORITY priority);

	void ExecuteCmdList(
		ID3D12CommandList ** ppCmdList,
		const UINT count);

	void Destroy();

	ID3D12CommandQueue* mp_cmdQueue;
private:

};