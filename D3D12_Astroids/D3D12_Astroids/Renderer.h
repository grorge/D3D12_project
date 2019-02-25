#pragma once
#include "D3DHeader.h"
#include "Object.h"
#include <iostream>
#include <thread>

#include "CommandQueue.h"
#include "CommandAllocator.h"
#include "CommandList.h"

#include "GraphicsPipelineState.h"

#include "UploadResource.h"
#include "DescriptorHeap.h"

#include <vector>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define NUM_SWAP_BUFFERS 5
#define NUM_CONST_BUFFERS 2
#define NUM_ALLOCATED_CONST_BUFFERS (NUM_CONST_BUFFERS * NUM_SWAP_BUFFERS)


#define CONST_COLOR_INDEX 0
#define CONST_TRANSLATION_INDEX 1

class Renderer
{
public:
	Renderer();
	~Renderer();

	void init(HWND hwnd);
	void startGame();
	void joinThreads();

	void clearAndReady();
	void update();
	//void render();
	void render(int threadID);

	ID3D12Device4*				device4 = nullptr;

private:
	void fillLists();
	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);
	void WaitForGpu(int threadID);

	void CreateDirect3DDevice();				
	void CreateCommandInterfacesAndSwapChain(HWND wndHandle);	
	void CreateFenceAndEventHandle();							
	void CreateRenderTargets();									
	void CreateViewportAndScissorRect();						
	void CreateShadersAndPiplelineState();									
	void CreateRootSignature();
	void CreateConstantBufferResources();
	void CreateDepthStencil();

	std::thread* frameThreads[NUM_SWAP_BUFFERS];
	int working = 0;
	bool running = true;

	MSG msg = { 0 };
	HWND hwnd;

	HRESULT hr;

	UINT backBufferIndex = 0;

	CommandQueue m_graphicsCmdQueue;
	CommandAllocator m_graphicsCmdAllocator[NUM_SWAP_BUFFERS];
	CommandList m_graphicsCmdList[NUM_SWAP_BUFFERS];

	GraphicsPipelineState m_pipelineState;

	DescriptorHeap m_constantBufferHeap;
	UploadResource m_constantBufferResource[NUM_ALLOCATED_CONST_BUFFERS];

	IDXGISwapChain4*			swapChain4 = nullptr;

	ID3D12Fence1*				fence = {};
	HANDLE						eventHandle = {};
	UINT64						fenceValue = {};
	//ID3D12Fence1*				fence[NUM_SWAP_BUFFERS] = {};
	//HANDLE						eventHandle[NUM_SWAP_BUFFERS] = {};
	//UINT64						fenceValue[NUM_SWAP_BUFFERS] = {};

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

	//-----------------------

	Object* object;
	std::vector<Object*> objectList;
};