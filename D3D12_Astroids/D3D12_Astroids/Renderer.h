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
#include <thread>


#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

#define NUM_SWAP_BUFFERS 2
#define NUM_CONST_BUFFERS 2
#define NUM_UAV_BUFFERS 5

#define CONST_COLOR_INDEX 0
#define CONST_TRANSLATION_INDEX 1

#define RUN_COMPUTESHADERS 1
#define RUN_THREADS 1



class Renderer
{
public:
	Renderer();
	~Renderer();
	void joinThreads();

	void init(HWND hwnd);
	void startGame();
	void initThreads();

	void ready();
	void update();
	void render();
	void tm_runFrame(unsigned int iD);
	void tm_copy();
	void tm_update();
	void tm_runCS();
	void RunComputeShader();

	ID3D12Device4*				device4 = nullptr;

private:
	void fillLists();
	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);
	void updateTranslation();
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

	void UploadData(void* data, const UINT byteWidth, Resource* pDest);

	HWND hwnd;

	HRESULT hr;

	UINT backBufferIndex = 0;

	std::thread* t_frame[NUM_SWAP_BUFFERS];
	std::thread* t_copyData;
	std::thread* t_update;
	bool running = false;
	unsigned int currThreadWorking = 0;

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
	ComputePipelineState m_computeStateCollision;
	ComputePipelineState m_computeStateClear;
	ComputePipelineState m_computeStateDraw;
	ComputePipelineState m_computeStateTranslation;

	DescriptorHeap m_constantBufferHeap;
	UploadResource m_constantBufferResource[NUM_CONST_BUFFERS];

	DescriptorHeap m_uavHeap;
	DefaultResource m_uavResourceFloat4;
	DefaultResource m_uavResourceIntArray;

	ID3D12Resource* m_texture;
	DescriptorHeap m_srvHeap;


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