#pragma once
#include "D3DHeader.h"
#include "Timestamps.h"

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


#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define NUM_SWAP_BUFFERS 6
#define NUM_CONST_BUFFERS 2
#define NUM_UAV_BUFFERS 6

#define CONST_COLOR_INDEX 0
#define CONST_TRANSLATION_INDEX 1

#define RUN_COMPUTESHADERS 1
#define RUN_THREADS 1
#define RUN_ONE_THREAD 0
#define RUN_SEQUENTIAL 0

#define RUN_TIME_STAMPS false
#define RUN_LOGICCOUNTER false

class Renderer
{
public:
	Renderer();
	~Renderer();
	void joinThreads();

	void init(HWND hwnd);
	void startGame();
	void initThreads();

	void tm_runFrame(unsigned int iD);
	void tm_copy();
	void tm_update();
	void tm_runCS();

	bool running = false;
	ID3D12Device4*				device4 = nullptr;

private:
	D3D12::D3D12Timer frameTimer;
	D3D12::D3D12Timer computeTimer;
	D3D12::D3D12Timer copyTimer;
	void timerPrint();
	unsigned int logicPerDraw = 0;

	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);
	void WaitForGpu(const int iD);
	void WaitForCompute();
	void WaitForCopy();


	void CreateKeyBoardInput();				
	void CreateDirect3DDevice();				
	void CreateCommandInterfacesAndSwapChain(HWND wndHandle);	
	void CreateFenceAndEventHandle();							
	void CreateRenderTargets();							
	void CreateShadersAndPiplelineState();									
	void CreateRootSignature();
	void CreateUnorderedAccessResources();

	HWND hwnd;

	HRESULT hr;

	UINT backBufferIndex = 0;

	std::thread* t_sequential;

	std::thread* t_frame[NUM_SWAP_BUFFERS];
	std::thread* t_copyData;
	std::thread* t_update;
	unsigned int currThreadWorking = 0;

	CommandQueue m_graphicsCmdQueue;
	CommandAllocator m_graphicsCmdAllocator[NUM_SWAP_BUFFERS];
	CommandList m_graphicsCmdList[NUM_SWAP_BUFFERS];

	CommandQueue m_computeCmdQueue;
	CommandAllocator m_computeCmdAllocator;
	CommandList m_computeCmdList;

	CommandQueue m_copyCmdQueue;
	CommandAllocator m_copyCmdAllocator;
	CommandList m_copyCmdList;

	ComputePipelineState m_computeState;
	ComputePipelineState m_computeStateKeyboard;
	ComputePipelineState m_computeStateCollision;
	ComputePipelineState m_computeStateClear;
	ComputePipelineState m_computeStateDraw;
	ComputePipelineState m_computeStateTranslation;
	ComputePipelineState m_computeStateBullet;
	ComputePipelineState m_computeStateBulletTranslation;

	DescriptorHeap m_uavHeap;

	ID3D12Resource* m_texture[NUM_SWAP_BUFFERS];
	DescriptorHeap m_srvHeap;


	UAVBuffer m_uavArray[NUM_UAV_BUFFERS];

	IDXGISwapChain4*			swapChain4 = nullptr;

	ID3D12Fence1*				fence[NUM_SWAP_BUFFERS];
	ID3D12Fence1*				copyFence = nullptr;
	ID3D12Fence1*				computeFence = nullptr;
	HANDLE						eventHandle[NUM_SWAP_BUFFERS];
	HANDLE						copyEventHandle = nullptr;
	HANDLE						computeEventHandle = nullptr;
	UINT64						fenceValue = 0;

	ID3D12Resource1*			renderTargets[NUM_SWAP_BUFFERS] = {};

	ID3D12RootSignature*		rootSignature = nullptr;
	
	ID3D12DescriptorHeap*		descriptorHeap[NUM_SWAP_BUFFERS] = {};
	
	KeyBoardInput*				keyboard = nullptr;

	void KeyboardInput();
	void KeyboardUpload();
	void KeyboardShader();
	void Translate();
	void Collision();
	void Fence();
	void CopyTranslation(ID3D12GraphicsCommandList* cmdList);
	void DrawShader(ID3D12GraphicsCommandList* cmdList);
	void CopyTexture(ID3D12GraphicsCommandList* cmdList);
	void PresentFrame(ID3D12GraphicsCommandList* cmdList);
};