#include "Renderer.h"

Renderer::Renderer()
{
	this->backBufferIndex = 0;
	this->threadCreated = false;
	this->prevBackBuff = NUM_SWAP_BUFFERS - 1;
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		this->frameReadyToClear[i] = true;
		this->present[i] = false;
		this->myTurn[i] = false;
	}
	myTurn[0] = true;
	this->savedSlot = 0;
	for (int i = 0; i < 64; i++)
	{
		this->savedTime[i] = 0.0f;
	}
}

Renderer::~Renderer()
{
	//Wait for GPU execution to be done and then release all interfaces.
	this->WaitForGpu(0);
	
	float totalTime = 0;
	for (int i = 0; i < 64; i++)
	{
		totalTime += this->savedTime[i];
	}
	totalTime /= 64;
	if (RUN_SEQUENTIAL)
	{
		printToDebug("\n -------- SEQUENCIAL FRAMETIME: ");
		printToDebug(totalTime);
		printToDebug("\n");
	}
	else
	{
		printToDebug("\n -------- PARALLEL FRAMETIME: ");
		printToDebug(totalTime);
		printToDebug("\n");
	}


	//this->WaitForGpu(m_computeCmdQueue());
	WaitForCompute();

	if (!RUN_SEQUENTIAL)
	{
		for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
			delete t_frame[i];
		delete t_update;
		delete t_copyData;
	}

	SafeRelease(&device4);		
	SafeRelease(&swapChain4);


	CloseHandle(this->computeEventHandle);
	CloseHandle(this->copyEventHandle);
	SafeRelease(&computeFence);
	SafeRelease(&copyFence);

	//SafeRelease(&renderTargetsHeap);
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		CloseHandle(eventHandle[i]);
		SafeRelease(&fence[i]);
		SafeRelease(&descriptorHeap[i]);
		SafeRelease(&renderTargets[i]);

		CloseHandle(mainEventHandle[i]);
		SafeRelease(&mainFence[i]);
	}

	SafeRelease(&rootSignature);

	for (int i = 0;  i < NUM_SWAP_BUFFERS; i++)
		SafeRelease(&m_texture[i]);

	delete this->keyboard;


}

void Renderer::joinThreads()
{
	this->running = false;
	
	int nrOfThreads = NUM_SWAP_BUFFERS;
	for (int i = 0; i < nrOfThreads; i++)
	{
		this->t_frame[i]->join();
	}
	this->t_copyData->detach();
	//this->t_copyData->join();
	this->t_update->detach();
	
	
}

void Renderer::init(HWND hwnd)
{
	this->hwnd = hwnd;

	this->CreateKeyBoardInput();
	
	this->CreateDirect3DDevice();					
	this->CreateCommandInterfacesAndSwapChain(hwnd);	
	this->CreateFenceAndEventHandle();						
	this->CreateRenderTargets();								
	this->CreateRootSignature();								
	this->CreateShadersAndPiplelineState();					
	this->CreateUnorderedAccessResources();

	if (RUN_TIME_STAMPS)
	{
		this->frameTimer.init(this->device4, 16);
		this->computeTimer.init(this->device4, 16);
		this->copyTimer.init(this->device4, 16);
	}

	//this->WaitForGpu(m_graphicsCmdQueue());
	WaitForGpu(0);
	
	//this->WaitForGpu(m_computeCmdQueue());
	WaitForCompute();

	//A quick sleep so that all resources are ready
	Sleep(2000);
}

void Renderer::startGame()
{
	srand((unsigned int)time(NULL));

	ConstantBuffer data = { 1.0f, 2.0f, 3.0f, 4.0f }; 
	TranslatonBuffer positionData[256 * OBJECT_MULTIPLIER];
	TranslatonBuffer directionData[256 * OBJECT_MULTIPLIER];

	for (int i = 0; i < 256 * OBJECT_MULTIPLIER; i++)
	{
		positionData[i].trans[0] = SCREEN_WIDTH * ((float)rand() / (float)RAND_MAX); // x-value
		positionData[i].trans[1] = SCREEN_HEIGHT * ((float)rand() / (float)RAND_MAX)/* * 100.0f - SCREEN_HEIGHT*/; // y-value
		positionData[i].trans[2] = 1.0f;
		directionData[i].trans[0] = 1.0f * (((float)rand() / (float)RAND_MAX) - 0.5f);
		directionData[i].trans[1] = 1.0f * (((float)rand() / (float)RAND_MAX) - 0.5f);
		directionData[i].trans[2] = 0.7f/* * ((float)rand() / (float)RAND_MAX) + 1.0f*/;
	}

	// Sets a default position for the player
	positionData[0].trans[0] = SCREEN_WIDTH / 2.0f;
	positionData[0].trans[1] = SCREEN_HEIGHT * (2.0f/3.0f);

	directionData[0].trans[2] = 2.0f; // speed

	TranslatonBuffer Bullet_positionData[128];
	TranslatonBuffer Bullet_directionData[128];

	for (int i = 0; i < 128; i++)
	{
		Bullet_positionData[i].trans[0] = -100.0f; // x-value
		Bullet_positionData[i].trans[1] = 100.0f; // y-value
		Bullet_positionData[i].trans[2] = 1.0f;

		Bullet_directionData[i].trans[0] = 0.0f;
		Bullet_directionData[i].trans[1] = -1.0f;
		Bullet_directionData[i].trans[2] = 0.0f;
	}
	Bullet_positionData[0].trans[0] = 0.0f; // x-value
	Bullet_positionData[0].trans[1] = 5.0f; // y-value
	Bullet_positionData[0].trans[2] = 0.0f;

	m_copyCmdAllocator()->Reset();
	m_copyCmdList()->Reset(m_copyCmdAllocator(), nullptr);

	m_uavArray[0].UploadData(&data, m_copyCmdList(), sizeof(data));
	//m_uavArray[1].UploadData(this->keyboard->keyBoardInt, m_copyCmdList(), sizeof(data));
	m_uavArray[2].UploadData(&positionData, m_copyCmdList(), sizeof(positionData));
	m_uavArray[3].UploadData(&directionData, m_copyCmdList(), sizeof(directionData));
	m_uavArray[4].UploadData(&Bullet_positionData, m_copyCmdList(), sizeof(Bullet_positionData));
	m_uavArray[5].UploadData(&Bullet_directionData, m_copyCmdList(), sizeof(Bullet_directionData));


	//Close the list to prepare it for execution.
	m_copyCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute1[] = { m_copyCmdList() };
	m_copyCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute1), listsToExecute1);

	//WaitForGpu(m_copyCmdQueue());
	WaitForCopy();
}

void Renderer::initThreads()
{
	this->running = true;

	
	int nrOfThreads = NUM_SWAP_BUFFERS;
	
	for (int i = 0; i < nrOfThreads; i++)
	{
		this->t_frame[i] = new std::thread([&](Renderer* rnd) { rnd->tm_runFrame(i); }, this);
	//	this->t_frame[i] = new std::thread([&](Renderer* rnd) { rnd->tm_runFrameDebug(i); }, this);
		while (!this->threadCreated){} 		//Make sure that the thread initialized before proceeding.
		this->threadCreated = false;		//Reset before the next loop.
	}
	this->t_copyData = new std::thread([&](Renderer* rnd) { rnd->tm_copy(); }, this);
	this->t_update = new std::thread([&](Renderer* rnd) { rnd->tm_runCS(); }, this);
	
}



void Renderer::tm_runFrame(const unsigned int iD)
{
	//Let the main thread know that the thread has finished it's initialization.
	this->threadCreated = true;

	while (this->running)
	{
		// Stall the thread until the main thread says it's okay to continue.
		if (myTurn[iD] && this->frameReadyToClear[iD])
		{
			myTurn[iD] = false;

			//printToDebug("Entered loop: ", (int)iD);

			//Reset before the new frame
			m_graphicsCmdAllocator[iD]()->Reset();
			m_graphicsCmdList[iD]()->Reset(m_graphicsCmdAllocator[iD](), nullptr);

			//Set root signature
			m_graphicsCmdList[iD]()->SetComputeRootSignature(rootSignature);
			ID3D12DescriptorHeap* descriptorHeaps[] = { m_uavHeap.mp_descriptorHeap };
			m_graphicsCmdList[iD]()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);
			m_graphicsCmdList[iD]()->SetComputeRootDescriptorTable(0, this->m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

			D3D12_GPU_DESCRIPTOR_HANDLE handle = this->m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE handlePrev = this->m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart();

			int prevIndex = 0;
			if (backBufferIndex != 0)
				prevIndex = (backBufferIndex - 1);
			else
				prevIndex = NUM_SWAP_BUFFERS - 1;
			handle.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (NUM_UAV_BUFFERS + backBufferIndex);
			handlePrev.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (NUM_UAV_BUFFERS + prevIndex);

			//Legacy stuff, could be done with only one. Kept so that we could clear another
			//"backbuffers texture".
			m_graphicsCmdList[iD]()->SetComputeRootDescriptorTable(1, handle); 
			m_graphicsCmdList[iD]()->SetComputeRootDescriptorTable(2, handle);

			if (RUN_TIME_STAMPS)
			{

				this->frameTimer.start(this->m_graphicsCmdList[iD](), 0); // Start frame timer

				this->frameTimer.start(this->m_graphicsCmdList[iD](), 1);
				this->ClearTexture(m_graphicsCmdList[iD](), iD);				//Copy Translation  //REMOVE?
				this->frameTimer.stop(this->m_graphicsCmdList[iD](), 1);

				this->frameTimer.start(this->m_graphicsCmdList[iD](), 2);
				this->DrawShaders(m_graphicsCmdList[iD]());				//Clear texture and draw
				this->frameTimer.stop(this->m_graphicsCmdList[iD](), 2);

				this->frameTimer.start(this->m_graphicsCmdList[iD](), 3);
				this->CopyTexture(m_graphicsCmdList[iD]());					//Copy the new texture to the backbuffer
				this->frameTimer.stop(this->m_graphicsCmdList[iD](), 3);

				this->frameTimer.stop(this->m_graphicsCmdList[iD](), 0); //Stop frame timer


				this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 0);
				this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 1);
				this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 2);
				this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 3);

				this->frameTimer.stop(this->m_graphicsCmdList[prevIndex](), 4);
				this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 4);
				this->frameTimer.start(this->m_graphicsCmdList[iD](), 4);
			}
			else
			{
				this->ClearTexture(m_graphicsCmdList[iD](), iD);
				this->DrawShaders(m_graphicsCmdList[iD]());
				this->SetResourceUavBarrier(m_graphicsCmdList[iD](), m_texture[iD]);
				this->CopyTexture(m_graphicsCmdList[iD]());
				this->SetResourceUavBarrier(m_graphicsCmdList[iD](), m_texture[iD]);
			}

			this->PresentFrame(m_graphicsCmdList[iD](), iD);
		}

	}
}

void Renderer::tm_copy()
{
	while (this->running)
	{
		if (this->input < 2)
		{
			//Reset the Copy command allocator and command list for the next pass.
			m_copyCmdAllocator()->Reset();
			m_copyCmdList()->Reset(m_copyCmdAllocator(), nullptr);

			//Gather keyboard input and upload it to the GPU.
			this->KeyboardInput();

			if (RUN_TIME_STAMPS)
			{
				copyTimer.start(m_copyCmdList(), 0);
				this->KeyboardUpload();
				copyTimer.stop(m_copyCmdList(), 0);

				copyTimer.resolveQueryToCPU(m_copyCmdList(), 0);
			}
			else
				this->KeyboardUpload();

			//Close the list to prepare it for execution.
			m_copyCmdList()->Close();

			//Execute the command list.
			ID3D12CommandList* listsToExecute0[] = { m_copyCmdList() };
			m_copyCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute0), listsToExecute0);

			//Wait for the Copy queue to finish execution of the command list.
			this->WaitForCopy();

			//Debug prints
			if (RUN_LOGICCOUNTER)
			{
				int privateLogic = (int)this->logicPerDraw;
				printToDebug(privateLogic);
				printToDebug("\n");
			}
			if (RUN_TIME_STAMPS)
				this->timerPrint();
			this->input++;
		}
	}
}

void Renderer::tm_update()
{
	while (this->running)
	{
		this->tm_copy();
		this->tm_runCS();
		this->tm_runFrame(0);

		if (RUN_TIME_STAMPS)
			this->timerPrint();
	}
}

void Renderer::tm_runCS()
{
	while (this->running)
	{
		if (this->logic < 2)
		{
			this->WaitForCompute();

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
				0,
				m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

			this->KeyboardShader();

			if (RUN_TIME_STAMPS)
			{
				this->computeTimer.start(this->m_computeCmdList(), 4);
				this->Translate();
				this->computeTimer.stop(this->m_computeCmdList(), 4);

				this->computeTimer.start(this->m_computeCmdList(), 5);
				this->Collision();
				this->computeTimer.stop(this->m_computeCmdList(), 5);

				this->computeTimer.resolveQueryToCPU(this->m_computeCmdList(), 4);
				this->computeTimer.resolveQueryToCPU(this->m_computeCmdList(), 5);
			}
			else
			{
				this->Translate();
				this->Collision();
			}


			m_computeCmdList()->Close();

			//Execute the command list.
			ID3D12CommandList* listsToExecute[] = { m_computeCmdList() };
			m_computeCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

			this->logicPerDraw++;
			this->logic++;
		}
	}
}

void Renderer::tm_main()
{
	if (present[this->backBufferIndex])
	{
		this->logic = 0;
		this->input = 0;
		this->prevBackBuff = this->backBufferIndex;
		//printToDebug("Presenting: ", this->prevBackBuff);
		swapChain4->Present(0, 0);
		this->backBufferIndex = this->swapChain4->GetCurrentBackBufferIndex();
		this->myTurn[backBufferIndex] = true;
		WaitForGpuMain(this->prevBackBuff);

		cpuTime = clock();
		float timeToPrint = (float)cpuTime - (float)cpuTimePrev;
		this->savedTime[this->savedSlot] = timeToPrint;
		this->savedSlot++;
		this->savedSlot = savedSlot % 64;
		//printToDebug("\n");
		//printToDebug((int)prev);
		//printToDebug("	Frametime: ", timeToPrint);
		cpuTimePrev = cpuTime; 
		if (RUN_LOGICCOUNTER)
		{
			int privateLogic = (int)this->logicPerDraw;
			printToDebug(privateLogic);
			printToDebug("\n");
		}
		logicPerDraw = 0;
		
		
		this->mtxFrameReadyToClear[this->prevBackBuff].lock();
		this->frameReadyToClear[this->prevBackBuff] = true;
		this->mtxFrameReadyToClear[this->prevBackBuff].unlock();
	
		this->mtxPresent[this->prevBackBuff].lock();
		present[this->prevBackBuff] = false;
		this->mtxPresent[this->prevBackBuff].unlock();
	}
}

void Renderer::sequentialFrame()
{
	/**************************KEYBOARD**********************************/
	//Reset the Copy command allocator and command list for the next pass.
	m_copyCmdAllocator()->Reset();
	m_copyCmdList()->Reset(m_copyCmdAllocator(), nullptr);

	//Gather keyboard input and upload it to the GPU.
	this->KeyboardInput();

	if (RUN_TIME_STAMPS)
	{
		copyTimer.start(m_copyCmdList(), 0);
		this->KeyboardUpload();
		copyTimer.stop(m_copyCmdList(), 0);

		copyTimer.resolveQueryToCPU(m_copyCmdList(), 0);
	}
	else
		this->KeyboardUpload();

	//Close the list to prepare it for execution.
	m_copyCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute0[] = { m_copyCmdList() };
	m_copyCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute0), listsToExecute0);

	//Wait for the Copy queue to finish execution of the command list.
	this->WaitForCopy();

	//Debug prints
	if (RUN_LOGICCOUNTER)
	{
		int privateLogic = (int)this->logicPerDraw;
		printToDebug(privateLogic);
		printToDebug("\n");
	}
	if (RUN_TIME_STAMPS)
		this->timerPrint();
	/*************************END KEYBOARD**************************/
	/*************************COMPUTE*******************************/
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
		0,
		m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	this->KeyboardShader();

	if (RUN_TIME_STAMPS)
	{
		this->computeTimer.start(this->m_computeCmdList(), 4);
		this->Translate();
		this->computeTimer.stop(this->m_computeCmdList(), 4);

		this->computeTimer.start(this->m_computeCmdList(), 5);
		this->Collision();
		this->computeTimer.stop(this->m_computeCmdList(), 5);

		this->computeTimer.resolveQueryToCPU(this->m_computeCmdList(), 4);
		this->computeTimer.resolveQueryToCPU(this->m_computeCmdList(), 5);
	}
	else
	{
		this->Translate();
		this->Collision();
	}


	m_computeCmdList()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { m_computeCmdList() };
	m_computeCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);
	this->WaitForCompute();
	/*******************************END COMPUTE****************************/
	/*******************************RENDER*********************************/
	int iD = swapChain4->GetCurrentBackBufferIndex();
	//printToDebug("Entered loop: ", (int)iD);

	//Reset before the new frame
	m_graphicsCmdAllocator[iD]()->Reset();
	m_graphicsCmdList[iD]()->Reset(m_graphicsCmdAllocator[iD](), nullptr);

	//Set root signature
	m_graphicsCmdList[iD]()->SetComputeRootSignature(rootSignature);
	ID3D12DescriptorHeap* descriptorHeaps0[] = { m_uavHeap.mp_descriptorHeap };
	m_graphicsCmdList[iD]()->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps0), descriptorHeaps0);
	m_graphicsCmdList[iD]()->SetComputeRootDescriptorTable(0, this->m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	D3D12_GPU_DESCRIPTOR_HANDLE handle = this->m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE handlePrev = this->m_uavHeap.mp_descriptorHeap->GetGPUDescriptorHandleForHeapStart();

	int prevIndex = 0;
	if (backBufferIndex != 0)
		prevIndex = (backBufferIndex - 1);
	else
		prevIndex = NUM_SWAP_BUFFERS - 1;
	handle.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (NUM_UAV_BUFFERS + backBufferIndex);
	handlePrev.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (NUM_UAV_BUFFERS + prevIndex);

	//Legacy stuff, could be done with only one. Kept so that we could clear another
	//"backbuffers texture".
	m_graphicsCmdList[iD]()->SetComputeRootDescriptorTable(1, handle);
	m_graphicsCmdList[iD]()->SetComputeRootDescriptorTable(2, handle);

	if (RUN_TIME_STAMPS)
	{

		this->frameTimer.start(this->m_graphicsCmdList[iD](), 0); // Start frame timer

		this->frameTimer.start(this->m_graphicsCmdList[iD](), 1);
		this->ClearTexture(m_graphicsCmdList[iD](), iD);				//Copy Translation  //REMOVE?
		this->frameTimer.stop(this->m_graphicsCmdList[iD](), 1);

		this->frameTimer.start(this->m_graphicsCmdList[iD](), 2);
		this->DrawShaders(m_graphicsCmdList[iD]());				//Clear texture and draw
		this->frameTimer.stop(this->m_graphicsCmdList[iD](), 2);

		this->frameTimer.start(this->m_graphicsCmdList[iD](), 3);
		this->CopyTexture(m_graphicsCmdList[iD]());					//Copy the new texture to the backbuffer
		this->frameTimer.stop(this->m_graphicsCmdList[iD](), 3);

		this->frameTimer.stop(this->m_graphicsCmdList[iD](), 0); //Stop frame timer


		this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 0);
		this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 1);
		this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 2);
		this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 3);

		this->frameTimer.stop(this->m_graphicsCmdList[prevIndex](), 4);
		this->frameTimer.resolveQueryToCPU(this->m_graphicsCmdList[iD](), 4);
		this->frameTimer.start(this->m_graphicsCmdList[iD](), 4);
	}
	else
	{
		this->ClearTexture(m_graphicsCmdList[iD](), iD);
		this->DrawShaders(m_graphicsCmdList[iD]());
		this->SetResourceUavBarrier(m_graphicsCmdList[iD](), m_texture[iD]);
		this->CopyTexture(m_graphicsCmdList[iD]());
		this->SetResourceUavBarrier(m_graphicsCmdList[iD](), m_texture[iD]);
	}

	//Close the list to prepare it for execution.
	m_graphicsCmdList[iD]()->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute2[] = { m_graphicsCmdList[iD]() };
	m_graphicsCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute2), listsToExecute2);

	this->prevBackBuff = this->backBufferIndex;
	//printToDebug("Presenting: ", this->prevBackBuff);
	swapChain4->Present(0, 0);
	this->backBufferIndex = this->swapChain4->GetCurrentBackBufferIndex();
	WaitForGpuMain(this->prevBackBuff);

	cpuTime = clock();
	float timeToPrint = (float)cpuTime - (float)cpuTimePrev;
	this->savedTime[this->savedSlot] = timeToPrint;
	this->savedSlot++;
	this->savedSlot %= 64;
	//printToDebug("\n");
	//printToDebug((int)prev);
	//printToDebug("	Frametime: ", timeToPrint);
	cpuTimePrev = cpuTime;
	if (RUN_LOGICCOUNTER)
	{
		int privateLogic = (int)this->logicPerDraw;
		printToDebug(privateLogic);
		printToDebug("\n");
	}
}

void Renderer::timerPrint()
{
	//get time in ms
	UINT64 queueFreq;
	m_graphicsCmdQueue()->GetTimestampFrequency(&queueFreq);
	float timestampToMs = (1.0f / queueFreq) * 1000.0f;

	D3D12::GPUTimestampPair timerFrame = this->frameTimer.getTimestampPair(0);
	D3D12::GPUTimestampPair timerCopyTran = this->frameTimer.getTimestampPair(1);
	D3D12::GPUTimestampPair timerDraw = this->frameTimer.getTimestampPair(2);
	D3D12::GPUTimestampPair timerCopyTex = this->frameTimer.getTimestampPair(3);
	D3D12::GPUTimestampPair test = this->frameTimer.getTimestampPair(4);
	D3D12::GPUTimestampPair timerTranslate = this->computeTimer.getTimestampPair(4);
	D3D12::GPUTimestampPair timerCollsion = this->computeTimer.getTimestampPair(5);
	D3D12::GPUTimestampPair timerCopyQueue = this->copyTimer.getTimestampPair(0);

	UINT64 dt = timerFrame.Stop - timerFrame.Start;
	float timeInMs = dt * timestampToMs;

	printToDebug("___DRAW___	");
	////printToDebug("\n");
	//printToDebug("	FillList:	");
	//printToDebug(timeInMs);
	////printToDebug("\n");

	//dt = timerCopyTran.Stop - timerCopyTran.Start;
	//timeInMs = dt * timestampToMs;
	//printToDebug("	Clear Texture:	");
	//printToDebug(timeInMs);
	////printToDebug("\n");

	dt = timerDraw.Stop - timerDraw.Start;
	timeInMs = dt * timestampToMs;
	printToDebug("	Draw on texture:	");
	printToDebug(timeInMs);
	//printToDebug("\n");

	//dt = timerCopyTex.Stop - timerCopyTex.Start;
	//timeInMs = dt * timestampToMs;
	//printToDebug("	Copy Texture:	");
	//printToDebug(timeInMs);
	////printToDebug("\n");


	printToDebug("	___LOGIC___	");
	//printToDebug("\n");
	dt = timerTranslate.Stop - timerTranslate.Start;
	timeInMs = dt * timestampToMs;
	printToDebug("	Translation:	");
	printToDebug(timeInMs);
	//printToDebug("\n");

	dt = timerCollsion.Stop - timerCollsion.Start;
	timeInMs = dt * timestampToMs;
	printToDebug("	Collison:	");
	printToDebug(timeInMs);
	//printToDebug("\n");

	//dt = test.Stop - test.Start;
	//timeInMs = dt * timestampToMs;
	//printToDebug("	test:	");
	//printToDebug(timeInMs);


	//printToDebug("	___COPY___	");
	////printToDebug("\n");
	//dt = timerCopyQueue.Stop - timerCopyQueue.Start;
	//timeInMs = dt * timestampToMs;
	//printToDebug("	Copy queue:	");
	//printToDebug(timeInMs);
	////printToDebug("\n");

	printToDebug("\n");
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

void Renderer::SetResourceUavBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrierDesc.UAV.pResource = resource;

	commandList->ResourceBarrier(1, &barrierDesc);
}

void Renderer::WaitForGpu(const int iD)
{
//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	this->m_graphicsCmdQueue()->Signal(this->fence[iD], fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->fence[iD]->GetCompletedValue() < fence)
	{
		this->fence[iD]->SetEventOnCompletion(fence, eventHandle[iD]);
		WaitForSingleObject(eventHandle[iD], INFINITE);
	}
}

void Renderer::WaitForGpuMain(const int iD)
{
//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	this->m_graphicsCmdQueue()->Signal(this->mainFence[iD], fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->mainFence[iD]->GetCompletedValue() < fence)
	{
		this->mainFence[iD]->SetEventOnCompletion(fence, mainEventHandle[iD]);
		WaitForSingleObject(mainEventHandle[iD], INFINITE);
	}
}

void Renderer::WaitForCompute()
{
	//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	m_computeCmdQueue()->Signal(this->computeFence, fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->computeFence->GetCompletedValue() < fence)
	{
		this->computeFence->SetEventOnCompletion(fence, computeEventHandle);
		WaitForSingleObject(computeEventHandle, INFINITE);
	}
}

void Renderer::WaitForCopy()
{
	//Signal and increment the fence value.
	const UINT64 fence = fenceValue;
	m_copyCmdQueue()->Signal(this->copyFence, fence);
	fenceValue++;

	//Wait until command queue is done.
	if (this->copyFence->GetCompletedValue() < fence)
	{
		this->copyFence->SetEventOnCompletion(fence, copyEventHandle);
		WaitForSingleObject(copyEventHandle, INFINITE);
	}
}

void Renderer::CreateKeyBoardInput()
{
	this->keyboard = new KeyBoardInput();
	keyboard->init();
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
	// this is a quick fix to allow timestamps on each queue
	D3D12_COMMAND_LIST_TYPE cmdCompute =
		RUN_TIME_STAMPS ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE;
	D3D12_COMMAND_LIST_TYPE cmdCopy = 
		RUN_TIME_STAMPS ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COPY;

	m_computeCmdQueue.Initialize(
		device4,
		cmdCompute,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL);

	m_computeCmdAllocator.Initialize(
		device4,
		cmdCompute);
	m_computeCmdAllocator()->SetName(L"compute Allocator");

	m_computeCmdList.Initialize(
		device4,
		m_computeCmdAllocator(),
		cmdCompute);

	m_copyCmdQueue.Initialize(
		device4,
		cmdCopy,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL);

	m_copyCmdAllocator.Initialize(
		device4,
		cmdCopy);
	m_copyCmdAllocator()->SetName(L"copy Allocator");

	m_copyCmdList.Initialize(
		device4,
		m_copyCmdAllocator(),
		cmdCopy);

	m_graphicsCmdQueue.Initialize(
		device4,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_PRIORITY_HIGH);

	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{

		m_graphicsCmdAllocator[i].Initialize(
			device4,
			D3D12_COMMAND_LIST_TYPE_DIRECT);
		//m_graphicsCmdAllocator[i]()->SetName(L"graphics Allocator");


		m_graphicsCmdList[i].Initialize(
			device4,
			m_graphicsCmdAllocator[i](),
			D3D12_COMMAND_LIST_TYPE_DIRECT);
	}


	m_graphicsCmdAllocator[0]()->SetName(L"graphics0 Allocator");
	m_graphicsCmdAllocator[1]()->SetName(L"graphics1 Allocator");

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
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		eventHandle[i] = CreateEvent(0, false, false, 0);
		device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->mainFence[i]));
		mainEventHandle[i] = CreateEvent(0, false, false, 0);
	}
	device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&copyFence));
	copyEventHandle = CreateEvent(0, false, false, 0);

	device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&computeFence));
	computeEventHandle = CreateEvent(0, false, false, 0);

	fenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	
}

void Renderer::CreateRenderTargets()
{
	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		hr = swapChain4->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
	}
}

void Renderer::CreateShadersAndPiplelineState()
{
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

	m_computeStateBullet.SetComputeShader("ComputeShaderBullet.hlsl");
	m_computeStateBullet.Compile(device4, rootSignature);
	m_computeStateBulletTranslation.SetComputeShader("ComputeShaderBulletTranslation.hlsl");
	m_computeStateBulletTranslation.Compile(device4, rootSignature);
}

void Renderer::CreateRootSignature()
{
	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  uavRanges;
	uavRanges.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRanges.NumDescriptors = NUM_UAV_BUFFERS + 1; //only one CB in this example
	uavRanges.BaseShaderRegister = 0; //register b0
	uavRanges.RegisterSpace = 0; //register(b0,space0);
	uavRanges.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE uavDt;
	uavDt.NumDescriptorRanges = 1;
	uavDt.pDescriptorRanges = &uavRanges;

	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  uavRanges2;
	uavRanges2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRanges2.NumDescriptors = 1; //only one CB in this example
	uavRanges2.BaseShaderRegister = NUM_UAV_BUFFERS + 1; //register b0
	uavRanges2.RegisterSpace = 0; //register(b0,space0);
	uavRanges2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE uavDt2;
	uavDt2.NumDescriptorRanges = 1;
	uavDt2.pDescriptorRanges = &uavRanges2;

	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  uavRanges3;
	uavRanges3.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRanges3.NumDescriptors = 1; //only one CB in this example
	uavRanges3.BaseShaderRegister = NUM_UAV_BUFFERS + 2; //register b0
	uavRanges3.RegisterSpace = 0; //register(b0,space0);
	uavRanges3.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE uavDt3;
	uavDt3.NumDescriptorRanges = 1;
	uavDt3.pDescriptorRanges = &uavRanges3;

	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[3];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = uavDt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable = uavDt2;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].DescriptorTable = uavDt3;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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

void Renderer::CreateUnorderedAccessResources()
{
	// Create Heap For All UAVs
	m_uavHeap.Initialize(
		device4,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		NUM_UAV_BUFFERS + NUM_SWAP_BUFFERS);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuAddress =
		m_uavHeap.mp_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Legacy buffer that was keept to not have to change the index in hte project
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

	// Position Buffer
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc2 = {};
	desc2.Format = DXGI_FORMAT_UNKNOWN;
	desc2.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

	desc2.Buffer.FirstElement = 0;
	desc2.Buffer.NumElements = 256 * OBJECT_MULTIPLIER;
	desc2.Buffer.StructureByteStride = sizeof(float) * 3;
	desc2.Buffer.CounterOffsetInBytes = 0;
	desc2.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	// Direction buffer
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc3 = {};
	desc3.Format = DXGI_FORMAT_UNKNOWN;
	desc3.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		
	desc3.Buffer.FirstElement = 0;
	desc3.Buffer.NumElements = 256 * OBJECT_MULTIPLIER;
	desc3.Buffer.StructureByteStride = sizeof(float) * 3;
	desc3.Buffer.CounterOffsetInBytes = 0;
	desc3.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	// Bullet position buffer
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc4 = {};
	desc4.Format = DXGI_FORMAT_UNKNOWN;
	desc4.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	desc4.Buffer.FirstElement = 0;
	desc4.Buffer.NumElements = 128;
	desc4.Buffer.StructureByteStride = sizeof(float) * 3;
	desc4.Buffer.CounterOffsetInBytes = 0;
	desc4.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	// Bullet direction buffer
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc5 = {};
	desc5.Format = DXGI_FORMAT_UNKNOWN;
	desc5.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	desc5.Buffer.FirstElement = 0;
	desc5.Buffer.NumElements = 128;
	desc5.Buffer.StructureByteStride = sizeof(float) * 3;
	desc5.Buffer.CounterOffsetInBytes = 0;
	desc5.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	const D3D12_UNORDERED_ACCESS_VIEW_DESC descArray[] = {
		desc0,
		desc1,
		desc2,
		desc3,
		desc4,
		desc5,
	};

	const UINT byteWidthArray[] = {
		32,
		1024,
		4096 * OBJECT_MULTIPLIER,
		4096 * OBJECT_MULTIPLIER,
		2048,
		2048,
	};

	const bool cpuWriteArray[] = {
		true,
		true,
		true,
		true,
		true,
		true,
	};

	const bool cpuReadArray[] = {
		true,
		false,
		true,
		false,
		false,
		false,
	};

	for (int i = 0; i < NUM_UAV_BUFFERS; i++)
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
	m_uavArray[0]()->SetName(L"Legacy Buffer");
	m_uavArray[1]()->SetName(L"Keyboard Buffer");
	m_uavArray[2]()->SetName(L"Position Buffer");
	m_uavArray[3]()->SetName(L"Direction Buffer");
	m_uavArray[4]()->SetName(L"Bullet position Buffer");
	m_uavArray[5]()->SetName(L"Bullet direction Buffer");

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
	
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		device4->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_texture[i])
		);

		D3D12_UNORDERED_ACCESS_VIEW_DESC description = {};
		description.Format = texDesc.Format;
		description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		device4->CreateUnorderedAccessView(m_texture[i], nullptr, &description, cpuAddress);
		cpuAddress.ptr += device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

void Renderer::KeyboardInput()
{
	// Update keyboard
	keyboard->readKeyboard();
}

void Renderer::KeyboardUpload()
{
	this->SetResourceUavBarrier(m_copyCmdList(), m_uavArray[1]());
	m_uavArray[1].UploadData(this->keyboard->keyBoardInt, m_copyCmdList(), sizeof(this->keyboard->keyBoardInt));

	// Dowload the position data from the GPU, this is to see the players state with the z-value
	// We dowload alot but we only need 1 float, this is to stress the system
	//m_uavArray[2].DownloadData(m_copyCmdList());
}

void Renderer::KeyboardShader()
{
	// Shader proccesing keyboard
	m_computeCmdList()->SetPipelineState(m_computeStateKeyboard.mp_pipelineState);
	m_computeCmdList()->Dispatch(32, 1, 1);
}

void Renderer::Translate()
{
	// Moves the objects
	m_computeCmdList()->SetPipelineState(m_computeStateTranslation.mp_pipelineState);
	m_computeCmdList()->Dispatch(256 * OBJECT_MULTIPLIER, 1, 1);
	// Moves the bullets
	m_computeCmdList()->SetPipelineState(m_computeStateBulletTranslation.mp_pipelineState);
	m_computeCmdList()->Dispatch(128, 1, 1);
}

void Renderer::Collision()
{
	// Shader looking for collision
	m_computeCmdList()->SetPipelineState(m_computeStateCollision.mp_pipelineState);
	m_computeCmdList()->Dispatch(256 * OBJECT_MULTIPLIER, 1, 1);
}

void Renderer::ClearTexture(ID3D12GraphicsCommandList * cmdList, const int iD)
{
	// Clear Texture
	cmdList->SetPipelineState(m_computeStateClear.mp_pipelineState);
	cmdList->Dispatch(SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	this->SetResourceUavBarrier(m_graphicsCmdList[iD](), m_texture[iD]);
}

void Renderer::DrawShaders(ID3D12GraphicsCommandList * cmdList)
{
	float* data = (float*)m_uavArray[2].GetData();

	//Draw astrooids
	cmdList->SetPipelineState(m_computeStateDraw.mp_pipelineState);
	cmdList->Dispatch(256 * OBJECT_MULTIPLIER, 1, 1);
	//Draw bullets
	cmdList->SetPipelineState(m_computeStateBullet.mp_pipelineState);
	cmdList->Dispatch(1, 1, 1);
	
}

void Renderer::CopyTexture(ID3D12GraphicsCommandList* cmdList)
{
	SetResourceTransitionBarrier(cmdList,
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,	//state before
		D3D12_RESOURCE_STATE_COPY_DEST		//state after
	);

	cmdList->CopyResource(
		renderTargets[backBufferIndex],
		m_texture[backBufferIndex]);

	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(cmdList,
		renderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_COPY_DEST,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);
}

void Renderer::PresentFrame(ID3D12GraphicsCommandList* cmdList, const int iD)
{
	//Close the list to prepare it for execution.
	cmdList->Close();

	while (iD != swapChain4->GetCurrentBackBufferIndex()){}
	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { cmdList };
	m_graphicsCmdQueue()->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);
	
	//printToDebug("Executed CmdList: ", iD);
	this->mtxFrameReadyToClear[iD].lock();
	this->frameReadyToClear[iD] = false;
	this->mtxFrameReadyToClear[iD].unlock();

	this->mtxPresent[iD].lock();
	present[iD] = true; //Let the main thread know that this frame is ready to be presented
	this->mtxPresent[iD].unlock();
}


