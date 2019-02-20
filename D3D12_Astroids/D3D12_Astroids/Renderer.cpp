#include "Renderer.h"


Renderer::Renderer()
{
	this->gameActive = true;
}

Renderer::~Renderer()
{
	//Wait for GPU execution to be done and then release all interfaces.
	WaitForGpu();
	CloseHandle(eventHandle);
	SafeRelease(&device4);		
	SafeRelease(&commandQueue);
	SafeRelease(&commandAllocator);
	SafeRelease(&commandList4);
	SafeRelease(&swapChain4);

	SafeRelease(&fence);

	SafeRelease(&renderTargetsHeap);
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SafeRelease(&descriptorHeap[i]);
		SafeRelease(&constantBufferResource[i]);
		SafeRelease(&renderTargets[i]);
	}

	SafeRelease(&rootSignature);
	SafeRelease(&pipeLineState);

	SafeRelease(&descriptorHeapConstBuffers);

	delete this->object;
	for (Object* obj : this->objectList)
	{
		delete obj;
	}

	SafeRelease(&dsDescriptorHeap);
	SafeRelease(&depthStencilBuffer);

	this->copyThread->join();
	this->computeThread->join();
	this->queueThread->join();

	//SafeRelease(&vertexBufferResource);
}

void Renderer::init(HWND hwnd)
{
	this->hwnd = hwnd;

	this->CreateDirect3DDevice(hwnd);					//2. Create Device

	this->CreateCommandInterfacesAndSwapChain(hwnd);	//3. Create CommandQueue and SwapChain

	this->CreateFenceAndEventHandle();						//4. Create Fence and Event handle

	this->CreateRenderTargets();								//5. Create render targets for backbuffer

	this->CreateViewportAndScissorRect();						//6. Create viewport and rect

	this->CreateRootSignature();								//7. Create root signature

	this->CreateShadersAndPiplelineState();					//8. Set up the pipeline state

	this->CreateConstantBufferResources();					//9. Create constant buffer data

	this->CreateDepthStencil();
	//CreateTriangleData();

	InitThreads();

	WaitForGpu();
}

void Renderer::startGame()
{
	this->object = new Object(this->device4, 1);

	for (int i = 0; i < 3; i++)
	{
		this->objectList.push_back(new Object(this->device4, i));
	}
}

void Renderer::ready()
{
	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	commandAllocator->Reset();
	commandList4->Reset(commandAllocator, pipeLineState);

	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(commandList4,
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeapConstBuffers };
	//ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap[backBufferIndex] };
	commandList4->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	//Set root signature
	commandList4->SetGraphicsRootSignature(rootSignature);

	//Set root descriptor table to index 0 in previously set root signature
	commandList4->SetGraphicsRootDescriptorTable(0, descriptorHeapConstBuffers->GetGPUDescriptorHandleForHeapStart());
	//commandList4->SetGraphicsRootDescriptorTable(0, descriptorHeap[backBufferIndex]->GetGPUDescriptorHandleForHeapStart());

	//Set necessary states.
	commandList4->RSSetViewports(1, &viewport);
	commandList4->RSSetScissorRects(1, &scissorRect);

	//Record commands.
	//Get the handle for the current render target used as back buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = renderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += renderTargetDescriptorSize * backBufferIndex;

	commandList4->OMSetRenderTargets(1, &cdh, true, nullptr);

	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	commandList4->ClearRenderTargetView(cdh, clearColor, 0, nullptr);
	this->commandList4->ClearDepthStencilView(this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}

void Renderer::update()
{
	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)

	this->backBufferIndex = swapChain4->GetCurrentBackBufferIndex();
	
	UINT instances = objectList.size();
	UINT byteWidth = sizeof(float) * 4;
	int i = 0;

	void* translationData = malloc(byteWidth * instances);
	void* colorData = malloc(byteWidth * instances);

	for (Object* obj : this->objectList)
	{
		obj->update();

		UINT offset = i * byteWidth;;

		memcpy(static_cast<char*>(translationData) + offset,
			&obj->GETTranslationBufferData(), byteWidth);
		memcpy(static_cast<char*>(colorData) + offset,
			&obj->GETColorBufferData(), byteWidth);

		i++;
	}

	//Update GPU memory
	void* mappedMem = nullptr;
	D3D12_RANGE writeRange = { 0, byteWidth * instances };
	this->constantBufferResource[CONST_TRANSLATION_INDEX]->Map(0, nullptr, &mappedMem);
	memcpy(mappedMem, translationData, byteWidth * instances);
	this->constantBufferResource[CONST_TRANSLATION_INDEX]->Unmap(0, &writeRange);
	
	this->constantBufferResource[CONST_COLOR_INDEX]->Map(0, nullptr, &mappedMem);
	memcpy(mappedMem, colorData, byteWidth * instances);
	this->constantBufferResource[CONST_COLOR_INDEX]->Unmap(0, &writeRange);



	//for (Object* obj : this->objectList)
	//{
	//
	//	//Update GPU memory
	//	void* mappedMem = nullptr;
	//	D3D12_RANGE readRange = { 0, 0 }; //We do not intend to read this resource on the CPU.
	//	if (SUCCEEDED(constantBufferResource[backBufferIndex]->Map(0, &readRange, &mappedMem)))
	//	{
	//		memcpy(mappedMem, &obj->GETConstBufferData(), sizeof(ConstantBuffer));

	//		D3D12_RANGE writeRange = { 0, sizeof(ConstantBuffer) };
	//		constantBufferResource[backBufferIndex]->Unmap(0, &writeRange);
	//	}
	//}

	free(translationData);
	free(colorData);
}

void Renderer::render()
{
	this->ready();
	
	this->fillLists();


	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(commandList4,
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	//Close the list to prepare it for execution.
	commandList4->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { commandList4 };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	swapChain4->Present1(0, 0, &pp);

	WaitForGpu(); //Wait for GPU to finish.
				  //NOT BEST PRACTICE, only used as such for simplicity.
}

void Renderer::fillLists()
{
	UINT instances = objectList.size();
	commandList4->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	for (Object* obj : this->objectList)
	{
		obj->addToCommList(this->commandList4);
		
	}

	commandList4->DrawInstanced(4, instances, 0, 0);
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

void Renderer::WaitForGpu()
{
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
//This is code implemented as such for simplicity. The cpu could for example be used
//for other tasks to prepare the next frame while the current one is being rendered.

//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	commandQueue->Signal(this->fence, fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->fence->GetCompletedValue() < fence)
	{
		this->fence->SetEventOnCompletion(fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
	}
}

void Renderer::CreateDirect3DDevice(HWND wndHandle)
{
#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	ID3D12Debug* debugController = nullptr;

#ifdef STATIC_LINK_DEBUGSTUFF
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease(debugController);
#else
	HMODULE mD3D12 = GetModuleHandle("D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	if (SUCCEEDED(f(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease(&debugController);
#endif
#endif

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
		HRESULT hr = S_OK;
		//Create the actual device.
		if (SUCCEEDED(hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device4))))
		{

		}

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
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	device4->CreateCommandQueue(&cqd, IID_PPV_ARGS(&commandQueue));

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	device4->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	//Create command list.
	device4->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator,
		nullptr,
		IID_PPV_ARGS(&commandList4));

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	commandList4->Close();

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
		commandQueue,
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
	////// Shader Compiles //////
	ID3DBlob* vertexBlob;
	D3DCompileFromFile(
		L"VertexShader.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"vs_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&vertexBlob,	// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);

	ID3DBlob* pixelBlob;
	D3DCompileFromFile(
		L"PixelShader.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"ps_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&pixelBlob,		// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	);

	////// Input Layout //////
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	////// Pipline State //////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	//Specify pipeline stages:
	gpsd.pRootSignature = rootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vertexBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = vertexBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(pixelBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = pixelBlob->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	device4->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&pipeLineState));
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
	//for (int i = 0; i < NUM_CONST_BUFFERS; i++)
	//{
	D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
	heapDescriptorDesc.NumDescriptors = NUM_CONST_BUFFERS;
	heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	device4->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&descriptorHeapConstBuffers));
	//}

	UINT constBuffersSize = device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = descriptorHeapConstBuffers->GetCPUDescriptorHandleForHeapStart();

	UINT cbSizeAligned = (sizeof(ConstantBuffer) + 255) & ~255;	// 256-byte aligned CB.

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1; //used when multi-gpu
	heapProperties.VisibleNodeMask = 1; //used when multi-gpu
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (int i = 0; i < NUM_CONST_BUFFERS; i++)
	{
		device4->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constantBufferResource[i])
		);

		constantBufferResource[i]->SetName(L"cb heap");

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = constantBufferResource[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = cbSizeAligned;
		device4->CreateConstantBufferView(&cbvDesc, cdh);

		cdh.ptr += constBuffersSize;
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
		IID_PPV_ARGS(&this->depthStencilBuffer)
	);

	this->dsDescriptorHeap->SetName(L"DepthStencil Resource Heap");

	this->device4->CreateDepthStencilView(this->depthStencilBuffer, &dsStencilViewDesc, this->dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

void Renderer::InitThreads()
{
	this->copyThread = new std::thread(&Renderer::CopyThreadFunc, this);
	this->computeThread = new std::thread(&Renderer::ComputeThreadFunc, this);
	this->queueThread = new std::thread(&Renderer::QueueThreadFunc, this);
}

void Renderer::CopyThreadFunc()
{
	while (this->gameActive)
	{

	}
}

void Renderer::ComputeThreadFunc()
{
	while (this->gameActive)
	{

	}
}

void Renderer::QueueThreadFunc()
{
	while (this->gameActive)
	{

	}
}