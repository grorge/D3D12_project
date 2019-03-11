#include "Renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
	//Wait for GPU execution to be done and then release all interfaces.
	this->WaitForGpu(m_graphicsCmdQueue());
	this->WaitForGpu(m_computeCmdQueue());

	CloseHandle(eventHandle);
	SafeRelease(&device4);		
	SafeRelease(&swapChain4);

	SafeRelease(&fence);

	SafeRelease(&renderTargetsHeap);
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SafeRelease(&descriptorHeap[i]);
		SafeRelease(&renderTargets[i]);
	}

	SafeRelease(&rootSignature);
	SafeRelease(&pipeLineState);

	delete this->object;
	for (Object* obj : this->objectList)
	{
		delete obj;
	}

	SafeRelease(&dsDescriptorHeap);
	SafeRelease(&depthStencilBuffer);

	SafeRelease(&m_texture);

	delete this->keyboard;
}

void Renderer::init(HWND hwnd)
{
	this->hwnd = hwnd;

	this->CreateKeyBoardInput();

	this->CreateDirect3DDevice();					//2. Create Device

	this->CreateCommandInterfacesAndSwapChain(hwnd);	//3. Create CommandQueue and SwapChain

	this->CreateFenceAndEventHandle();						//4. Create Fence and Event handle

	this->CreateRenderTargets();								//5. Create render targets for backbuffer

	this->CreateViewportAndScissorRect();						//6. Create viewport and rect

	this->CreateRootSignature();								//7. Create root signature

	this->CreateShadersAndPiplelineState();					//8. Set up the pipeline state

	this->CreateConstantBufferResources();					//9. Create constant buffer data

	this->CreateUnorderedAccessResources();

	this->CreateDepthStencil();
	//CreateTriangleData();


	this->WaitForGpu(m_graphicsCmdQueue());
	this->WaitForGpu(m_computeCmdQueue());
}

void Renderer::startGame()
{
	srand(time(NULL));

	Vertex triangleVertices[4] =
	{
		-1.0f, -1.0f, -1.0f,//v0 pos
		0.0f, 1.0f,			//v0 uv
		//1.0f, 0.0f, 0.0f,	//v0 color

		-1.0f, 1.0f, -1.0f,	//v1
		0.0f, 0.0f,			//v1 uv
		//0.0f, 1.0f, 0.0f,	//v1 color

		1.0f, -1.0f, -1.0f,  //v2
		1.0f, 1.0f,			//v2 uv
		//0.0f, 0.0f, 1.0f,	//v2 color

		1.0f, 1.0f, -1.0f,  //v3
		1.0f, 0.0f,			//v3 uv
		//1.0f, 1.0f, 0.0f	//v3 color
	};

	this->object = new Object(this->device4, 1);

	for (int i = 0; i < 3; i++)
	{
		this->objectList.push_back(new Object(this->device4, i));
	}

	const UINT byteWidth = sizeof(triangleVertices);
	this->UploadData(triangleVertices, byteWidth, &this->objectList[0]->m_resource);

	ConstantBuffer data = { 1.0f, 2.0f, 3.0f, 4.0f }; 
	TranslatonBuffer positionData[256];
	TranslatonBuffer directionData[256];

	for (int i = 0; i < 256; i++)
	{
		//float temp = (((float)rand() / (float)RAND_MAX) - 0.5f );
		positionData[i].trans[0] = SCREEN_WIDTH * ((float)rand() / (float)RAND_MAX);
		positionData[i].trans[1] = SCREEN_HEIGHT * ((float)rand() / (float)RAND_MAX);
		positionData[i].trans[2] = 1.0f;
		directionData[i].trans[0] = 1.0f * (((float)rand() / (float)RAND_MAX) - 0.5f);
		directionData[i].trans[1] = 1.0f * (((float)rand() / (float)RAND_MAX) - 0.5f);
		directionData[i].trans[2] = 1.0f;
	}

	// Sets a default position for the player
	//positionData[0].trans[0] = 300.0f;
	//positionData[0].trans[1] = 300.0f;
	//positionData[0].trans[2] = 1.0f;


	m_copyCmdAllocator()->Reset();
	m_copyCmdList()->Reset(m_copyCmdAllocator(), nullptr);

	m_uavArray[0].UploadData(&data, m_copyCmdList());
	m_uavIntArray.UploadData(this->keyboard->keyBoardInt, m_copyCmdList());
	m_uavArray[2].UploadData(&positionData, m_copyCmdList());
	m_uavArray[3].UploadData(&directionData, m_copyCmdList());


	//Close the list to prepare it for execution.
	m_copyCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute1[] = { m_copyCmdList() };
	m_copyCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute1), listsToExecute1);

	WaitForGpu(m_copyCmdQueue());
}

void Renderer::ready()
{
	WaitForGpu(m_graphicsCmdQueue()); //Wait for GPU to finish.
				  //NOT BEST PRACTICE, only used as such for simplicity.

	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	m_graphicsCmdAllocator()->Reset();
	m_graphicsCmdList()->Reset(m_graphicsCmdAllocator(), m_graphicsState.mp_pipelineState);

	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(m_graphicsCmdList(),
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	////Set constant buffer descriptor heap
	//ID3D12DescriptorHeap* descriptorHeaps[] = { m_constantBufferHeap.mp_descriptorHeap };
	//m_graphicsCmdList()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	////Set root signature
	//m_graphicsCmdList()->SetGraphicsRootSignature(rootSignature);

	////Set root descriptor table to index 0 in previously set root signature
	//m_graphicsCmdList()->SetGraphicsRootDescriptorTable(
	//	0, 
	//	m_constantBufferHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	////Set necessary states.
	//m_graphicsCmdList()->RSSetViewports(1, &viewport);
	//m_graphicsCmdList()->RSSetScissorRects(1, &scissorRect);

	//Record commands.
	//Get the handle for the current render target used as back buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += renderTargetDescriptorSize * backBufferIndex;

	/*D3D12_CPU_DESCRIPTOR_HANDLE cpuAccessDSV = this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_graphicsCmdList()->OMSetRenderTargets(1, &cdh, true, &cpuAccessDSV);
*/
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_graphicsCmdList()->ClearRenderTargetView(cdh, clearColor, 0, nullptr);
	/*m_graphicsCmdList()->ClearDepthStencilView(
		this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 
		D3D12_CLEAR_FLAG_DEPTH, 
		1.0f, 
		0, 
		0, 
		nullptr);*/
}

void Renderer::update()
{
	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)

	this->backBufferIndex = swapChain4->GetCurrentBackBufferIndex();
	
	UINT instances = (UINT)objectList.size();
	UINT byteWidth = (UINT)sizeof(float) * 4;
	int i = 0;

	char* translationData	= (char*)malloc(byteWidth * instances);
	char* colorData			= (char*)malloc(byteWidth * instances);

	for (Object* obj : this->objectList)
	{
		obj->update();

		UINT offset = i * byteWidth;

		memcpy(
			translationData + offset,
			&obj->GETTranslationBufferData(), 
			byteWidth);

		memcpy(
			colorData + offset,
			&obj->GETColorBufferData(), 
			byteWidth);

		i++;
	}

	m_constantBufferResource[0].SetData(colorData);
	m_constantBufferResource[1].SetData(translationData);


	if (RUN_COMPUTESHADERS)
	{
		// Update keyboard
		//if (keyboard->readKeyboard())
		keyboard->readKeyboard();

		m_copyCmdAllocator()->Reset();
		m_copyCmdList()->Reset(m_copyCmdAllocator(), nullptr);

		m_uavArray[1].UploadData(this->keyboard->keyBoardInt, m_copyCmdList());

		//Close the list to prepare it for execution.
		m_copyCmdList()->Close();

		//Execute the command list.
		ID3D12CommandList* listsToExecute0[] = { m_copyCmdList() };
		m_copyCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute0), listsToExecute0);

		WaitForGpu(m_copyCmdQueue());
	}

	free(translationData);
	free(colorData);
}

void Renderer::render()
{
	this->ready();
	
	this->fillLists();


	/*ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.mp_descriptorHeap };
	m_graphicsCmdList()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
*/
	//// Set Root Argument, Index 1
	//m_graphicsCmdList()->SetGraphicsRootDescriptorTable(
	//	2,
	//	m_srvHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	//
	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(m_graphicsCmdList(),
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_COPY_DEST		//state after
	);

	m_graphicsCmdList()->CopyResource(
		renderTargets[backBufferIndex],
		m_texture);

	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(m_graphicsCmdList(),
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_COPY_DEST,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	//Close the list to prepare it for execution.
	m_graphicsCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { m_graphicsCmdList() };
	m_graphicsCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	swapChain4->Present1(0, 0, &pp);
}

void Renderer::RunComputeShader()
{
	WaitForGpu(m_computeCmdQueue());

	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	m_computeCmdAllocator()->Reset();
	m_computeCmdList()->Reset(m_computeCmdAllocator(), nullptr);

	//Set root signature
	m_computeCmdList()->SetComputeRootSignature(rootSignature);


	ID3D12DescriptorHeap* descriptorHeaps[] = { m_uavHeap.mp_descriptorHeap };
	m_computeCmdList()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);


	// Set Root Argument, Index 1
	m_computeCmdList()->SetComputeRootDescriptorTable(
		1,
		m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// Clear Texture
	m_computeCmdList()->SetPipelineState(m_computeStateClear.mp_pipelineState);
	m_computeCmdList()->Dispatch(SCREEN_WIDTH, SCREEN_HEIGHT, 1);

	// Shader proccesing keyboard
	m_computeCmdList()->SetPipelineState(m_computeStateKeyboard.mp_pipelineState);
	m_computeCmdList()->Dispatch(32, 1, 1);

	// Moves the objects
	m_computeCmdList()->SetPipelineState(m_computeStateTranslation.mp_pipelineState);
	m_computeCmdList()->Dispatch(256, 1, 1);

	// Shader looking for collision
	m_computeCmdList()->SetPipelineState(m_computeStateCollision.mp_pipelineState);
	m_computeCmdList()->Dispatch(64, 1, 1);

	// Testing shader
	m_computeCmdList()->SetPipelineState(m_computeState.mp_pipelineState);
	m_computeCmdList()->Dispatch(3, 1, 1);

	//m_computeCmdList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	m_computeCmdList()->SetPipelineState(m_computeStateDraw.mp_pipelineState);
	m_computeCmdList()->Dispatch(1, 1, 1);
	//m_computeCmdList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));

	m_computeCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { m_computeCmdList() };
	m_computeCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	WaitForGpu(m_computeCmdQueue());
/*
	m_copyCmdAllocator()->Reset();
	m_copyCmdList()->Reset(m_copyCmdAllocator(), nullptr);

	m_uavArray[0].DownloadData(m_copyCmdList());


	//Close the list to prepare it for execution.
	m_copyCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute1[] = { m_copyCmdList() };
	m_copyCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute1), listsToExecute1);

	WaitForGpu(m_copyCmdQueue());

	float* data = (float*)m_uavArray[0].GetData();

	printToDebug("\n__Data__\nx: ");
	printToDebug((int)data[0]);
	printToDebug("\ny: ");
	printToDebug((int)data[1]);
	printToDebug("\nz: ");
	printToDebug((int)data[2]);
	printToDebug("\nw: ");
	printToDebug((int)data[3]);

	this->updateTranslation();

	//this->objectList[0]->translation.values[0] = (float)data[0];
	//this->objectList[0]->translation.values[1] = (float)data[1];

	Sleep(1000);
*/
}

void Renderer::fillLists()
{
	UINT instances = 1; //(UINT)objectList.size();
	m_graphicsCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	objectList[0]->addToCommList(m_graphicsCmdList());

	//m_graphicsCmdList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	//m_graphicsCmdList()->DrawInstanced(4, instances, 0, 0);
	//m_graphicsCmdList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON));
}

void Renderer::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
	D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}

void Renderer::updateTranslation()
{
	float* data = (float*)m_uavArray[0].GetData();

	for (int i = 0; i < 3/* objectList.size()*/; i++)
	{
		objectList[i]->translation.values[0] = data[0 + (i * 4)];
		objectList[i]->translation.values[1] = data[1 + (i * 4)];
	}
}

void Renderer::WaitForGpu(ID3D12CommandQueue* queue)
{
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
//This is code implemented as such for simplicity. The cpu could for example be used
//for other tasks to prepare the next frame while the current one is being rendered.

//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	queue->Signal(this->fence, fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->fence->GetCompletedValue() < fence)
	{
		this->fence->SetEventOnCompletion(fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
	}
}

void Renderer::CreateKeyBoardInput()
{
	this->keyboard = new KeyBoardInput();
	keyboard->init();
	//keyboard->init(&this->m_uavResourceIntArray);
	//keyboard->printKeyboard();
	//keyboard->keyBoardInt[3] = 1;
}

void Renderer::CreateDirect3DDevice()
{
	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6*	factory = nullptr;
	IDXGIAdapter1*	adapter = nullptr;
	//First a factory is created to iterate through the adapters available.
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device5), nullptr)))
		{
			break;
		}

		SafeRelease(&adapter);
	}

	if (adapter)
	{
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device4));
		SafeRelease(&adapter);
	}
	else
	{
		//Create warp device if no adapter was found.
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device4));
	}

	SafeRelease(&factory);
}

void Renderer::CreateCommandInterfacesAndSwapChain(HWND wndHandle)
{
	m_computeCmdQueue.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL);

	m_computeCmdAllocator.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_COMPUTE);

	m_computeCmdList.Initialize(
		device4,
		m_computeCmdAllocator(),
		D3D12_COMMAND_LIST_TYPE_COMPUTE);

	m_copyCmdQueue.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_COPY,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL);

	m_copyCmdAllocator.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_COPY);

	m_copyCmdList.Initialize(
		device4,
		m_copyCmdAllocator(),
		D3D12_COMMAND_LIST_TYPE_COPY);


	m_graphicsCmdQueue.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL);

	m_graphicsCmdAllocator.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_DIRECT);

	m_graphicsCmdList.Initialize(
		device4,
		m_graphicsCmdAllocator(),
		D3D12_COMMAND_LIST_TYPE_DIRECT);

	IDXGIFactory5*	factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = SCREEN_WIDTH;
	scDesc.Height = SCREEN_HEIGHT;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	if (SUCCEEDED(hr = factory->CreateSwapChainForHwnd(
		m_graphicsCmdQueue(),
		wndHandle,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1)))
	{
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain4))))
		{
			swapChain4->Release();
		}
	}

	SafeRelease(&factory);
}

void Renderer::CreateFenceAndEventHandle()
{
	device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	fenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	eventHandle = CreateEvent(0, false, false, 0);
}

void Renderer::CreateRenderTargets()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	HRESULT hr = device4->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&renderTargetsHeap));

	//Create resources for the render targets.
	renderTargetDescriptorSize = device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		hr = swapChain4->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
		device4->CreateRenderTargetView(renderTargets[n], nullptr, cdh);
		cdh.ptr += renderTargetDescriptorSize;
	}
}

void Renderer::CreateViewportAndScissorRect()
{
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)SCREEN_WIDTH;
	viewport.Height = (float)SCREEN_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = (long)0;
	scissorRect.right = (long)SCREEN_WIDTH;
	scissorRect.top = (long)0;
	scissorRect.bottom = (long)SCREEN_HEIGHT;
}

void Renderer::CreateShadersAndPiplelineState()
{
	////// Input Layout //////
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"	, 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		//{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	m_graphicsState.SetInputLayout(inputLayoutDesc);
	m_graphicsState.SetVertexShader("VertexShader.hlsl");
	m_graphicsState.SetPixelShader("PixelShader.hlsl");
	m_graphicsState.SetPrimitiveToplogy(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_graphicsState.Compile(device4, rootSignature);

	m_computeState.SetComputeShader("ComputeShader.hlsl");
	m_computeState.Compile(device4, rootSignature);

	m_computeStateKeyboard.SetComputeShader("ComputeShaderKeyboard.hlsl");
	m_computeStateKeyboard.Compile(device4, rootSignature);

	m_computeStateClear.SetComputeShader("ComputeShaderClear.hlsl");
	m_computeStateClear.Compile(device4, rootSignature);

	m_computeStateDraw.SetComputeShader("ComputeShaderDraw.hlsl");
	m_computeStateDraw.Compile(device4, rootSignature);

	m_computeStateCollision.SetComputeShader("ComputeShaderCollision.hlsl");
	m_computeStateCollision.Compile(device4, rootSignature);

	m_computeStateTranslation.SetComputeShader("ComputeShaderTranslation.hlsl");
	m_computeStateTranslation.Compile(device4, rootSignature);
}

void Renderer::CreateRootSignature()
{
	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges;
	dtRanges.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges.NumDescriptors = NUM_CONST_BUFFERS; //only one CB in this example
	dtRanges.BaseShaderRegister = 0; //register b0
	dtRanges.RegisterSpace = 0; //register(b0,space0);
	dtRanges.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = 1;
	dt.pDescriptorRanges = &dtRanges;


	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  uavRanges;
	uavRanges.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRanges.NumDescriptors = NUM_UAV_BUFFERS; //only one CB in this example
	uavRanges.BaseShaderRegister = 0; //register b0
	uavRanges.RegisterSpace = 0; //register(b0,space0);
	uavRanges.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE uavDt;
	uavDt.NumDescriptorRanges = 1;
	uavDt.pDescriptorRanges = &uavRanges;

	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  srvRanges;
	srvRanges.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRanges.NumDescriptors = 1; //only one CB in this example
	srvRanges.BaseShaderRegister = 0; //register t0
	srvRanges.RegisterSpace = 0; //register(t0,space0);
	srvRanges.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE srvDt;
	srvDt.NumDescriptorRanges = 1;
	srvDt.pDescriptorRanges = &srvRanges;


	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[3];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable = uavDt;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].DescriptorTable = srvDt;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	ID3DBlob* sBlob;
	D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr);

	device4->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
}

void Renderer::CreateConstantBufferResources()
{
	UINT cbSize = (sizeof(ConstantBuffer) + 255) & ~255;	// 256-byte aligned CB.

	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.SizeInBytes = cbSize;

	m_constantBufferHeap.Initialize(
		device4,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		NUM_CONST_BUFFERS);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuAddress = 
		m_constantBufferHeap.mp_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (unsigned int i = 0; i < NUM_CONST_BUFFERS; i++)
	{
		m_constantBufferResource[i].Initialize(
			device4,
			cbSize,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_FLAG_NONE);

		desc.BufferLocation = m_constantBufferResource[i].mp_resource->GetGPUVirtualAddress();
		
		device4->CreateConstantBufferView(&desc, cpuAddress);
		cpuAddress.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

void Renderer::CreateUnorderedAccessResources()
{
	// Create Heap For All UAVs
	m_uavHeap.Initialize(
		device4,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		NUM_UAV_BUFFERS);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuAddress =
		m_uavHeap.mp_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc0 = {};
	desc0.Format = DXGI_FORMAT_R32_FLOAT;
	desc0.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

	desc0.Buffer.FirstElement = 0;
	desc0.Buffer.NumElements = 4;
	desc0.Buffer.StructureByteStride = 0;
	desc0.Buffer.CounterOffsetInBytes = 0;
	desc0.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc1 = {};
	desc1.Format = DXGI_FORMAT_R32_UINT;
	desc1.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	   
	desc1.Buffer.FirstElement = 0;
	desc1.Buffer.NumElements = 256;
	desc1.Buffer.StructureByteStride = 0;
	desc1.Buffer.CounterOffsetInBytes = 0;
	desc1.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	/*m_uavIntArray.Initialize(device4, uavSize, true, false);

	device4->CreateUnorderedAccessView(
		this->m_uavIntArray(),
		NULL,
		&desc1,
		cpuAddress);
*/

	// Position Buffer
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc2 = {};
	desc2.Format = DXGI_FORMAT_UNKNOWN;
	desc2.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

	desc2.Buffer.FirstElement = 0;
	desc2.Buffer.NumElements = 256;
	desc2.Buffer.StructureByteStride = sizeof(float) * 3;
	desc2.Buffer.CounterOffsetInBytes = 0;
	desc2.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	// Direction buffer
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc3 = {};
	desc3.Format = DXGI_FORMAT_UNKNOWN;
	desc3.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		
	desc3.Buffer.FirstElement = 0;
	desc3.Buffer.NumElements = 256;
	desc3.Buffer.StructureByteStride = sizeof(float) * 3;
	desc3.Buffer.CounterOffsetInBytes = 0;
	desc3.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	const D3D12_UNORDERED_ACCESS_VIEW_DESC descArray[] = {
		desc0,
		desc1,
		desc2,
		desc3,
	};

	const UINT byteWidthArray[] = {
		32,
		1024,
		4096,
		4096,
	};

	const bool cpuWriteArray[] = {
		true,
		true,
		true,
		true,
	};

	const bool cpuReadArray[] = {
		true,
		false,
		false,
		false,
	};

	for (int i = 0; i < NUM_UAV_BUFFERS - 1; i++)
	{
		m_uavArray[i].Initialize(
			device4,
			byteWidthArray[i],
			cpuWriteArray[i],
			cpuReadArray[i]);

		device4->CreateUnorderedAccessView(
			m_uavArray[i](),
			NULL,
			&descArray[i],
			cpuAddress);

		cpuAddress.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Describe and create a shader resource view (SRV) heap for the texture.
	m_srvHeap.Initialize(
		device4,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		1);

	//TEXTURE
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.MipLevels = 1;
	texDesc.Alignment = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = SCREEN_WIDTH;
	texDesc.Height = SCREEN_HEIGHT;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	texDesc.DepthOrArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	device4->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_texture)
		);

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device4->CreateShaderResourceView(m_texture, &srvDesc, m_srvHeap.mp_descriptorHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_UNORDERED_ACCESS_VIEW_DESC description = {};
	description.Format = texDesc.Format;
	description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	//description.Texture2D.MipSlice = 1;
	//description.Texture2D.PlaneSlice = 1;

	device4->CreateUnorderedAccessView(m_texture, nullptr, &description, cpuAddress);
}

void Renderer::CreateDepthStencil()
{
	// --------------------- Depth Stencil

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = { };
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = this->device4->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&this->dsDescriptorHeap));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsStencilViewDesc = { };
	dsStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptClearValue = { };
	depthOptClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptClearValue.DepthStencil.Depth = 1.0f;
	depthOptClearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES dsHeapProp = {};
	dsHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	dsHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	dsHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	dsHeapProp.CreationNodeMask = 0;
	dsHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC dsResourceDesc = { };
	dsResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsResourceDesc.Alignment = 0;// 65536; //Wah?
	dsResourceDesc.Width = (UINT)SCREEN_WIDTH;
	dsResourceDesc.Height = (UINT)SCREEN_HEIGHT;
	dsResourceDesc.DepthOrArraySize = 1;
	dsResourceDesc.MipLevels = 1;
	dsResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsResourceDesc.SampleDesc = DXGI_SAMPLE_DESC{ 1,0 };
	dsResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	hr = this->device4->CreateCommittedResource(
		&dsHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&dsResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptClearValue,
		IID_PPV_ARGS(&this->depthStencilBuffer));

	this->dsDescriptorHeap->SetName(L"DepthStencil Resource Heap");

	this->device4->CreateDepthStencilView(
		this->depthStencilBuffer, 
		&dsStencilViewDesc, 
		this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::UploadData(void * data, const UINT byteWidth, Resource * pDest)
{
	UploadResource upload;
	upload.Initialize(
		device4,
		byteWidth,
		D3D12_HEAP_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_FLAG_NONE);

	upload.SetData(data);

	m_copyCmdAllocator()->Reset();
	m_copyCmdList()->Reset(m_copyCmdAllocator(), nullptr);

	m_copyCmdList()->CopyResource(pDest->mp_resource, upload.mp_resource);

	//Close the list to prepare it for execution.
	m_copyCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute1[] = { m_copyCmdList() };
	m_copyCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute1), listsToExecute1);
	
	WaitForGpu(m_copyCmdQueue());
	upload.Destroy();
}
