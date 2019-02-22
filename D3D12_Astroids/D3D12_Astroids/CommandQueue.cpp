#include "CommandQueue.h"


CommandQueue::CommandQueue()
{
	mp_cmdQueue = nullptr;
}

CommandQueue::~CommandQueue()
{
	Destroy();
}

void CommandQueue::Initialize(
	ID3D12Device * pDevice,
	const D3D12_COMMAND_LIST_TYPE type,
	const D3D12_COMMAND_QUEUE_PRIORITY priority)
{
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};

		// Possible Effect On performance?
		desc.Priority = priority;

		desc.Type = type;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mp_cmdQueue));
	}
}

void CommandQueue::ExecuteCmdList(
	ID3D12CommandList ** ppCmdList,
	const UINT count)
{
	mp_cmdQueue->ExecuteCommandLists(count, ppCmdList);
}

void CommandQueue::Destroy()
{
	if (mp_cmdQueue)
	{
		mp_cmdQueue->Release();
		mp_cmdQueue = nullptr;
	}
}

ID3D12CommandQueue * CommandQueue::operator()()
{
	return mp_cmdQueue;
}
