#include "Object.h"

Object::Object(ID3D12Device4* device4)
{
	//this->device4Reff = device4Reff;

	this->CreateTriangleData(device4);
}

Object::~Object()
{
	SafeRelease(&this->vertexBufferResource);
}

void Object::addToCommList(ID3D12GraphicsCommandList3 * commandList4)
{
	commandList4->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList4->DrawInstanced(3, 1, 0, 0);
}

void Object::update(UINT bbIndex)
{
	//Update constant buffer
	for (int i = 0; i < 3; i++)
	{
		constantBufferCPU.colorChannel[i] += 0.0001f * (i + 1);
		if (constantBufferCPU.colorChannel[i] > 1)
		{
			constantBufferCPU.colorChannel[i] = 0;
		}
	}
}

void Object::CreateTriangleData(ID3D12Device4* device4)
{
	Vertex triangleVertices[3] =
	{
		0.0f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		0.5f, -0.5f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		-0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f	//v2 color
	};

	//Note: using upload heaps to transfer static data like vert buffers is not 
	//recommended. Every time the GPU needs it, the upload heap will be marshalled 
	//over. Please read up on Default Heap usage. An upload heap is used here for 
	//code simplicity and because there are very few vertices to actually transfer.
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = sizeof(triangleVertices);
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Creates both a resource and an implicit heap, such that the heap is big enough
	//to contain the entire resource and the resource is mapped to the heap. 
	device4->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferResource));

	vertexBufferResource->SetName(L"vb heap");

	//Copy the triangle data to the vertex buffer.
	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 }; //We do not intend to read this resource on the CPU.
	vertexBufferResource->Map(0, &range, &dataBegin);
	memcpy(dataBegin, triangleVertices, sizeof(triangleVertices));
	vertexBufferResource->Unmap(0, nullptr);

	//Initialize vertex buffer view, used in the render call.
	vertexBufferView.BufferLocation = vertexBufferResource->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = sizeof(triangleVertices);
}

void Object::CreateConstantBufferResources()
{
}
