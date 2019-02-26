#include "ComputePipelineState.h"


ComputePipelineState::ComputePipelineState()
{
}

ComputePipelineState::~ComputePipelineState()
{
	Destroy();
}

void ComputePipelineState::Compile(
	ID3D12Device4 * pDevice,
	ID3D12RootSignature* pRootSignature)
{
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		
		desc.CS.pShaderBytecode = mp_computeShader->GetBufferPointer();
		desc.CS.BytecodeLength = mp_computeShader->GetBufferSize();
		desc.NodeMask = 0;
		desc.pRootSignature = pRootSignature;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mp_pipelineState));
	}
}

void ComputePipelineState::SetComputeShader(const std::string filepath)
{
	std::wstring path = std::wstring(filepath.begin(), filepath.end());

	D3DCompileFromFile(
		path.c_str(), // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"cs_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&mp_computeShader,	// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);
}

void ComputePipelineState::Destroy()
{
	SafeRelease(&mp_pipelineState);
	SafeRelease(&mp_computeShader);
}
