#include "Renderer.h"

void UploadResourceToDefault(
	Resource* pDest,
	Resource* pSrc,
	ID3D12GraphicsCommandList* pCmdList)
{
	/*

		if pSrc is an uploadResource, transitions may not be allowed
		https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_heap_type

	*/

	D3D12_RESOURCE_BARRIER open;
	open.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	open.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	open.Transition.pResource = pDest->mp_resource;
	open.Transition.StateBefore = pDest->m_currentState;
	open.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	open.Transition.Subresource = 0;


	D3D12_RESOURCE_BARRIER close;
	close.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	close.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	close.Transition.pResource = pDest->mp_resource;
	close.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	close.Transition.StateAfter = pDest->m_currentState;
	close.Transition.Subresource = 0;


	pCmdList->ResourceBarrier(1, &open);
	pCmdList->CopyResource(pDest->mp_resource, pSrc->mp_resource);
	pCmdList->ResourceBarrier(1, &close);
}

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
	//Wait for GPU execution to be done and then release all interfaces.
	//WaitForGpu(0);
	

	this->joinThreads();

	SafeRelease(&device4);		
	SafeRelease(&swapChain4);

	CloseHandle(eventHandle);
	SafeRelease(&fence);

	SafeRelease(&renderTargetsHeap);
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		this->m_graphicsCmdAllocator[i].Destroy();
		this->m_graphicsCmdList[i].Destroy();
		SafeRelease(&descriptorHeap[i]);
		SafeRelease(&renderTargets[i]);
	}
	for (int i = 0; i < NUM_ALLOCATED_CONST_BUFFERS; i++)
	{
		m_constantBufferResource[i].Destroy();
		//SafeRelease(&m_constantBufferResource[i]);
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
}

void Renderer::init(HWND hwnd)
{
	this->working = 0;

	this->hwnd = hwnd;

	this->CreateDirect3DDevice();					//2. Create Device

	this->CreateCommandInterfacesAndSwapChain(hwnd);	//3. Create CommandQueue and SwapChain

	this->CreateFenceAndEventHandle();						//4. Create Fence and Event handle

	this->CreateRenderTargets();								//5. Create render targets for backbuffer

	this->CreateViewportAndScissorRect();						//6. Create viewport and rect

	this->CreateRootSignature();								//7. Create root signature

	this->CreateShadersAndPiplelineState();					//8. Set up the pipeline state

	this->CreateConstantBufferResources();					//9. Create constant buffer data

	this->CreateDepthStencil();
	//CreateTriangleData();

	//this->WaitForGpu(0);


}

void Renderer::startGame()
{
	Vertex triangleVertices[4] =
	{
		-0.5f, -0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		-0.5f, 0.5f, 1.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f,	//v2 color

		0.5f, 0.5f, -1.0f, //v3
		0.0f, 0.0f, 1.0f	//v3 color
	};

	const UINT byteWidth = sizeof(triangleVertices);

	UploadResource upload;
	upload.Initialize(
		device4,
		byteWidth,
		D3D12_HEAP_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ);

	upload.SetData(triangleVertices);

	this->object = new Object(this->device4, 1);

	for (int i = 0; i < 3; i++)
	{
		this->objectList.push_back(new Object(this->device4, i));
	}
	
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		m_graphicsCmdAllocator[i]()->Reset();
		m_graphicsCmdList[i]()->Reset(m_graphicsCmdAllocator[i](), nullptr);

		UploadResourceToDefault(
			&this->objectList[0]->m_resource,
			&upload,
			m_graphicsCmdList[i]());

		//Close the list to prepare it for execution.
		m_graphicsCmdList[i]()->Close();

		//Execute the command list.
		ID3D12CommandList* listsToExecute[] = { m_graphicsCmdList[i]() };
		m_graphicsCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	}

	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		//this->frameThreads[i] = new std::thread(&Renderer::render, i, this );
		this->frameThreads[i] = new std::thread([&](Renderer* rnd) { rnd->render(i); }, this);
	}

	WaitForGpu(0);
	upload.Destroy();
}

void Renderer::joinThreads()
{
	this->running = false;
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		frameThreads[i]->join();
		delete frameThreads[i];
	}

}

void Renderer::clearAndReady()
{
	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	m_graphicsCmdAllocator[this->backBufferIndex]()->Reset();
	m_graphicsCmdList[this->backBufferIndex]()->Reset(m_graphicsCmdAllocator[this->backBufferIndex](), m_pipelineState.mp_pipelineState);

	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(m_graphicsCmdList[this->backBufferIndex](),
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_constantBufferHeap.mp_descriptorHeap };
	m_graphicsCmdList[this->backBufferIndex]()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	//Set root signature
	m_graphicsCmdList[this->backBufferIndex]()->SetGraphicsRootSignature(rootSignature);

	//Set root descriptor table to index 0 in previously set root signature
	m_graphicsCmdList[this->backBufferIndex]()->SetGraphicsRootDescriptorTable(
		0, 
		m_constantBufferHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//Set necessary states.
	m_graphicsCmdList[this->backBufferIndex]()->RSSetViewports(1, &viewport);
	m_graphicsCmdList[this->backBufferIndex]()->RSSetScissorRects(1, &scissorRect);

	//Record commands.
	//Get the handle for the current render target used as back buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += renderTargetDescriptorSize * backBufferIndex;

	m_graphicsCmdList[this->backBufferIndex]()->OMSetRenderTargets(1, &cdh, true, nullptr);

	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_graphicsCmdList[this->backBufferIndex]()->ClearRenderTargetView(cdh, clearColor, 0, nullptr);
	m_graphicsCmdList[this->backBufferIndex]()->ClearDepthStencilView(this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}

void Renderer::update()
{
	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)

	//this->backBufferIndex = swapChain4->GetCurrentBackBufferIndex();
	
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

	free(translationData);
	free(colorData);
}

//void Renderer::clearAndReady()
//{
//	//Command list allocators can only be reset when the associated command lists have
//	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
//	commandAllocator[this->backBufferIndex]->Reset();
//	commandList4[this->backBufferIndex]->Reset(commandAllocator[this->backBufferIndex], pipeLineState);
//
//	//Indicate that the back buffer will be used as render target.
//	SetResourceTransitionBarrier(commandList4[this->backBufferIndex],
//		renderTargets[backBufferIndex],
//		D3D12_RESOURCE_STATE_PRESENT,		//state before
//		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
//	);
//	//Set constant buffer descriptor heap
//	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeapConstBuffers };
//	commandList4[this->backBufferIndex]->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
//
//	//Set root signature
//	commandList4[this->backBufferIndex]->SetGraphicsRootSignature(rootSignature);
//
//	//Set root descriptor table to index 0 in previously set root signature
//	commandList4[this->backBufferIndex]->SetGraphicsRootDescriptorTable(0, descriptorHeapConstBuffers->GetGPUDescriptorHandleForHeapStart());
//	
//	//Set necessary states.
//	commandList4[this->backBufferIndex]->RSSetViewports(1, &viewport);
//	commandList4[this->backBufferIndex]->RSSetScissorRects(1, &scissorRect);
//
//	//Record commands.
//	//Get the handle for the current render target used as back buffer.
//	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
//	cdh.ptr += renderTargetDescriptorSize * backBufferIndex;
//
//	commandList4[this->backBufferIndex]->OMSetRenderTargets(1, &cdh, true, &this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//
//	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
//	commandList4[this->backBufferIndex]->ClearRenderTargetView(cdh, clearColor, 0, nullptr);
//	this->commandList4[this->backBufferIndex]->ClearDepthStencilView(this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//
//}


void Renderer::render(int threadID)
{
	while (this->running)
	{
		//while (threadID != 0)	{}
		while (threadID != swapChain4->GetCurrentBackBufferIndex())	{}

		printToDebug("ID: ", threadID);

		this->backBufferIndex = swapChain4->GetCurrentBackBufferIndex();

		this->update();

		this->clearAndReady();

		this->fillLists();

		//Indicate that the back buffer will now be used to present.
		SetResourceTransitionBarrier(m_graphicsCmdList[this->backBufferIndex](),
			renderTargets[backBufferIndex],
			D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
			D3D12_RESOURCE_STATE_PRESENT		//state after
		);

		//Close the list to prepare it for execution.
		m_graphicsCmdList[this->backBufferIndex]()->Close();

		//Execute the command list.
		ID3D12CommandList* listsToExecute[] = { m_graphicsCmdList[this->backBufferIndex]() };
		m_graphicsCmdQueue.ExecuteCmdList(listsToExecute, ARRAYSIZE(listsToExecute));


		// Stops the thread until the previous thread is done with present
		//while (this->working != threadID/* || (working == 4 && threadID == 0)*/) {}

		//Present the frame.
		DXGI_PRESENT_PARAMETERS pp = {};
		swapChain4->Present1(0, 0, &pp);


		//printToDebug("BBI: ", swapChain4->GetCurrentBackBufferIndex());

		WaitForGpu(threadID); //Wait for GPU to finish.
					  //NOT BEST PRACTICE, only used as such for simplicity.
	}
}

void Renderer::fillLists()
{
	UINT instances = (UINT)objectList.size();
	m_graphicsCmdList[this->backBufferIndex]()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	objectList[0]->addToCommList(m_graphicsCmdList[this->backBufferIndex]());

	m_graphicsCmdList[this->backBufferIndex]()->DrawInstanced(4, instances, 0, 0);
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

void Renderer::WaitForGpu(int threadID)
{
	//int i = threadID;
	int i = 0;
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
//This is code implemented as such for simplicity. The cpu could for example be used
//for other tasks to prepare the next frame while the current one is being rendered.

//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	m_graphicsCmdQueue()->Signal(this->fence, fence);
	fenceValue++;
	//Wait until command queue is done.
	//int test = this->fence->GetCompletedValue();
	//if (fence > test)
	if (this->fence->GetCompletedValue() < fence)
	{
		this->fence->SetEventOnCompletion(fence, eventHandle);
		//printToDebug("Start Wait: ", threadID);
		WaitForSingleObject(eventHandle, 1);
		//printToDebug("Done Wait: ", threadID);
		//working++;
		//working %= (NUM_SWAP_BUFFERS);
		//printToDebug("working: ", this->working);
	}
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
	m_graphicsCmdQueue.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL);

	//Create command list.
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		m_graphicsCmdAllocator[i].Initialize(
			device4,
			D3D12_COMMAND_LIST_TYPE_DIRECT);


		m_graphicsCmdList[i].Initialize(
			device4,
			m_graphicsCmdAllocator[i](),
			D3D12_COMMAND_LIST_TYPE_DIRECT);
		//Command lists are created in the recording state. Since there is nothing to
		//record right now and the main loop expects it to be closed, we close it.
		m_graphicsCmdList[i]()->Close();
	}


	IDXGIFactory5*	factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
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
	if (SUCCEEDED(factory->CreateSwapChainForHwnd(
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
	//for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	for (int i = 0; i < 1; i++)
	{
		device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		fenceValue = 1;
		//Create an event handle to use for GPU synchronization.
		eventHandle = CreateEvent(0, false, false, 0);
	}
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
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	m_pipelineState.SetInputLayout(inputLayoutDesc);
	m_pipelineState.SetVertexShader("VertexShader.hlsl");
	m_pipelineState.SetPixelShader("PixelShader.hlsl");
	m_pipelineState.SetPrimitiveToplogy(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_pipelineState.Compile(device4, rootSignature);
}

void Renderer::CreateRootSignature()
{
	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges[2];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 1; //only one CB in this example
	dtRanges[0].BaseShaderRegister = 0; //register b0
	dtRanges[0].RegisterSpace = 0; //register(b0,space0);
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	dtRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[1].NumDescriptors = 1; //only one CB in this example
	dtRanges[1].BaseShaderRegister = 1; //register b0
	dtRanges[1].RegisterSpace = 0; //register(b0,space0);
	dtRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[1];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

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
		NUM_ALLOCATED_CONST_BUFFERS);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuAddress = 
		m_constantBufferHeap.mp_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//Create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (int i = 0; i < NUM_ALLOCATED_CONST_BUFFERS; i++)
	{
		m_constantBufferResource[i].Initialize(
			device4,
			cbSize,
			D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ);

		desc.BufferLocation = m_constantBufferResource[i].mp_resource->GetGPUVirtualAddress();
		
		device4->CreateConstantBufferView(&desc, cpuAddress);
		cpuAddress.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
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
