#include "CommandAllocator.h"

CommandAllocator::CommandAllocator()
{
	mp_cmdAllocator = nullptr;
}

CommandAllocator::~CommandAllocator()
{
	Destroy();
}

void CommandAllocator::Initialize(
	ID3D12Device4 * pDevice,
	const D3D12_COMMAND_LIST_TYPE type)
{
	HRESULT hr = pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&mp_cmdAllocator));
}

void CommandAllocator::Destroy()
{
	if (mp_cmdAllocator)
	{
		mp_cmdAllocator->Release();
		mp_cmdAllocator = nullptr;
	}
}

ID3D12CommandAllocator * CommandAllocator::operator()()
{
	return mp_cmdAllocator;
}
