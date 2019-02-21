#pragma once

#include "PipelineState.h"

class ComputePipelineState : public Pipelinestate
{
public:
	ComputePipelineState();
	~ComputePipelineState();

	void Compile(
		ID3D12Device4* pDevice,
		ID3D12RootSignature* pRootSignature) override;

	void SetComputeShader(const std::string filepath);

	void Destroy() override;

private:
	ID3DBlob* mp_computeShader;
};
