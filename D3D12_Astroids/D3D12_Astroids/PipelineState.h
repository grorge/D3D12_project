#pragma once

#include "D3DHeader.h"
#include <string>

class Pipelinestate
{

public:

public:
	ID3D12PipelineState* mp_pipelineState;

	virtual void Compile(
		ID3D12Device4* pDevice,
		ID3D12RootSignature* pRootSignature) = 0;

	virtual void Destroy() = 0;

};