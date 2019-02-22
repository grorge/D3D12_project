#pragma once

#include "PipelineState.h"

class GraphicsPipelineState : public Pipelinestate
{
public:
	GraphicsPipelineState();
	~GraphicsPipelineState();

	void Compile(
		ID3D12Device4* pDevice,
		ID3D12RootSignature* pRootSignature) override;

	void SetPrimitiveToplogy(const D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);
	void SetInputLayout(const D3D12_INPUT_LAYOUT_DESC layout);
	void SetVertexShader(const std::string filepath);
	void SetPixelShader(const std::string filepath);

	void Destroy() override;

private:
	D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topology;
	D3D12_INPUT_LAYOUT_DESC m_layout;
	ID3DBlob* mp_vertexShader;
	ID3DBlob* mp_pixelShader;

};