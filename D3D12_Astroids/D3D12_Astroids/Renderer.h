#pragma once
#include "D3DHeader.h"
#include "Object.h"

#include <vector>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define NUM_SWAP_BUFFERS 2
#define NUM_CONST_BUFFERS 2

#define CONST_COLOR_INDEX 0
#define CONST_TRANSLATION_INDEX 1

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

private:
	void fillLists();
	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);
	void WaitForGpu();

	void CreateDirect3DDevice(HWND wndHandle);				
	void CreateCommandInterfacesAndSwapChain(HWND wndHandle);	
	void CreateFenceAndEventHandle();							
	void CreateRenderTargets();									
	void CreateViewportAndScissorRect();						
	void CreateShadersAndPiplelineState();									
	void CreateRootSignature();
	void CreateConstantBufferResources();
	void CreateDepthStencil();

	MSG msg = { 0 };
	HWND hwnd;

	HRESULT hr;

	UINT backBufferIndex = 0;

	ID3D12Device4*				device4 = nullptr;
	ID3D12GraphicsCommandList3*	commandList4 = nullptr;
	ID3D12CommandQueue*			commandQueue = nullptr;
	ID3D12CommandAllocator*		commandAllocator = nullptr;
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
	ID3D12DescriptorHeap*		descriptorHeapConstBuffers = {};
	ID3D12Resource1*			constantBufferResource[NUM_CONST_BUFFERS] = {};


	ID3D12DescriptorHeap*		dsDescriptorHeap = {};
	ID3D12Resource*				depthStencilBuffer = {};

	//-----------------------

	Object* object;
	std::vector<Object*> objectList;
};