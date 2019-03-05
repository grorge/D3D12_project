#pragma once
#include "D3DHeader.h"
#include "Object.h"

#include "CommandQueue.h"
#include "CommandAllocator.h"
#include "CommandList.h"

#include "GraphicsPipelineState.h"
#include "ComputePipelineState.h"

#include "DescriptorHeap.h"

#include "KeyBoardInput.h"

#include "UAVBuffer.h"

#include <vector>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define NUM_SWAP_BUFFERS 2
#define NUM_CONST_BUFFERS 2
#define NUM_UAV_BUFFERS 2

#define CONST_COLOR_INDEX 0
#define CONST_TRANSLATION_INDEX 1

#define RUN_COMPUTESHADERS 0



class Renderer
{
public:
	Renderer();
	~Renderer();

	void init(HWND hwnd);
	void startGame();

	void ready();
	void update();
	void render();
	void RunComputeShader();

	ID3D12Device4*				device4 = nullptr;

private:
	void fillLists();
	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);
	void WaitForGpu(ID3D12CommandQueue* queue);


	void CreateKeyBoardInput();				
	void CreateDirect3DDevice();				
	void CreateCommandInterfacesAndSwapChain(HWND wndHandle);	
	void CreateFenceAndEventHandle();							
	void CreateRenderTargets();									
	void CreateViewportAndScissorRect();						
	void CreateShadersAndPiplelineState();									
	void CreateRootSignature();
	void CreateConstantBufferResources();
	void CreateUnorderedAccessResources();
	void CreateDepthStencil();
	void CreateTex2DCompute();

	void UploadData(void* data, const UINT byteWidth, Resource* pDest);

	HWND hwnd;

	HRESULT hr;

	UINT backBufferIndex = 0;

	CommandQueue m_graphicsCmdQueue;
	CommandAllocator m_graphicsCmdAllocator;
	CommandList m_graphicsCmdList;

	CommandQueue m_computeCmdQueue;
	CommandAllocator m_computeCmdAllocator;
	CommandList m_computeCmdList;

	CommandQueue m_copyCmdQueue;
	CommandAllocator m_copyCmdAllocator;
	CommandList m_copyCmdList;

	GraphicsPipelineState m_graphicsState;
	ComputePipelineState m_computeState;
	ComputePipelineState m_computeStateKeyboard;
	ComputePipelineState m_computeStateDraw;

	DescriptorHeap m_constantBufferHeap;
	UploadResource m_constantBufferResource[NUM_CONST_BUFFERS];

	DescriptorHeap m_uavHeap;
	DefaultResource m_uavResourceFloat4;
	DefaultResource m_uavResourceIntArray;
	DefaultResource m_uavResourceDraw;

	ID3D12Resource* m_texture;
	ID3D12Resource* texUploadHeap;
	ID3D12DescriptorHeap* m_srvHeap;
	ID3D12DescriptorHeap* m_samplerHeap;


	UAVBuffer m_uavArray[NUM_UAV_BUFFERS];

	UAVBuffer m_uavFloat4, m_uavIntArray;

	IDXGISwapChain4*			swapChain4 = nullptr;

	ID3D12Fence1*				fence = nullptr;
	HANDLE						eventHandle = nullptr;
	UINT64						fenceValue = 0;

	ID3D12DescriptorHeap*		renderTargetsHeap = nullptr;
	ID3D12Resource1*			renderTargets[NUM_SWAP_BUFFERS] = {};
	UINT						renderTargetDescriptorSize = 0;

	D3D12_VIEWPORT				viewport = {};
	D3D12_RECT					scissorRect = {};

	ID3D12RootSignature*		rootSignature = nullptr;
	ID3D12PipelineState*		pipeLineState = nullptr;
	
	ID3D12DescriptorHeap*		descriptorHeap[NUM_SWAP_BUFFERS] = {};

	ID3D12DescriptorHeap*		dsDescriptorHeap = {};
	ID3D12Resource*				depthStencilBuffer = {};
	
	KeyBoardInput*				keyboard = nullptr;

	//-----------------------

	Object* object;
	std::vector<Object*> objectList;
};