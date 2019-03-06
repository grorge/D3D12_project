#include "GraphicsPipelineState.h"



GraphicsPipelineState::GraphicsPipelineState()
{
}

GraphicsPipelineState::~GraphicsPipelineState()
{
	Destroy();
}

void GraphicsPipelineState::Compile(
	ID3D12Device4 * pDevice,
	ID3D12RootSignature* pRootSignature)
{
	{
		D3D12_DEPTH_STENCIL_DESC dsDesc = { };
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		dsDesc.StencilEnable = FALSE;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

		desc.pRootSignature = pRootSignature;

		desc.PrimitiveTopologyType = m_topology;
		desc.InputLayout = m_layout;

		desc.VS.pShaderBytecode = mp_vertexShader->GetBufferPointer();
		desc.VS.BytecodeLength = mp_vertexShader->GetBufferSize();

		desc.PS.pShaderBytecode = mp_pixelShader->GetBufferPointer();
		desc.PS.BytecodeLength = mp_pixelShader->GetBufferSize();

		//Specify render target and depthstencil usage.
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.NumRenderTargets = 1;

		desc.SampleDesc.Count = 1;
		desc.SampleMask = UINT_MAX;

		desc.DepthStencilState = dsDesc;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		//Specify rasterizer behaviour.
		desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

		//Specify blend descriptions.
		D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
			false, false,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
			desc.BlendState.RenderTarget[i] = defaultRTdesc;

		HRESULT hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&mp_pipelineState));
		int dank = 5;
	}
}

void GraphicsPipelineState::SetPrimitiveToplogy(const D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
{
	m_topology = topology;
}

void GraphicsPipelineState::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC layout)
{
	m_layout = layout;
}

void GraphicsPipelineState::SetVertexShader(const std::string filepath)
{
	std::wstring path = std::wstring(filepath.begin(), filepath.end());

	D3DCompileFromFile(
		path.c_str(), // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"vs_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&mp_vertexShader,	// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
}

void GraphicsPipelineState::SetPixelShader(const std::string filepath)
{
	std::wstring path = std::wstring(filepath.begin(), filepath.end());

	D3DCompileFromFile(
		path.c_str(), // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"ps_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&mp_pixelShader,	// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
}

void GraphicsPipelineState::Destroy()
{
	SafeRelease(&mp_pipelineState);
	SafeRelease(&mp_vertexShader);
	SafeRelease(&mp_pixelShader);
}
