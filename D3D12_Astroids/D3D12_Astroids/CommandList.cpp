#include "CommandList.h"


CommandList::CommandList()
{
	mp_cmdList = nullptr;
}

CommandList::~CommandList()
{
	Destroy();
}

void CommandList::Initialize(
	ID3D12Device4 * pDevice,
	ID3D12CommandAllocator* pAllocator,
	const D3D12_COMMAND_LIST_TYPE type)
{
	HRESULT hr = pDevice->CreateCommandList(0, type, pAllocator, NULL, IID_PPV_ARGS(&mp_cmdList));
	mp_cmdList->Close();
}

void CommandList::Destroy()
{
	if (mp_cmdList)
	{
		mp_cmdList->Release();
		mp_cmdList = nullptr;
	}
}
