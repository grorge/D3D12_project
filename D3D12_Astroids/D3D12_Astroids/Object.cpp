#include "Object.h"

Object::Object(
	ID3D12Device4* device4, 
	int test)
{
	//this->device4Reff = device4Reff;

	this->CreateTriangleData(device4);

	//this->translation = { 0.5f, 1.0f, 0.0001f, 1.0f };
	//this->translation = { ((float)test / 3.0f) * 2.0f - 0.5f, ((float)test / 3.0f) * 2.0f - 1.0f, ((float)test / 3.0f) * 2.0f - 1.0f, 1.0f };
	//this->color = { ((float)test / 3.0f) * 2.0f - 0.5f, ((float)test / 3.0f) * 2.0f - 1.0f, 0.0001f };

	this->translation = { 1.0f, 1.0f, 1.0f };
	this->color = { 1.0f, 0.0f, 0.0f };

}

Object::~Object()
{
	SafeRelease(&this->vertexBufferResource);
	m_resource.Destroy();
}

void Object::addToCommList(ID3D12GraphicsCommandList3 * commandList4)
{
	commandList4->IASetVertexBuffers(0, 1, &vertexBufferView);
	//commandList4->DrawInstanced(3, 1, 0, 0);
}

void Object::update()
{
	//Update constant buffer
	for (int i = 0; i < 3; i++)
	{
		color.values[i] += 0.0001f * (i + 1);
		if (color.values[i] > 1)
		{
			color.values[i] = 0;
		}
	}

	if (this->translation.values[1] > 1.0f)
		this->translation.values[1] = -1.0f;
	else					/////1
		this->translation.values[1] += 0.001f;
}

void Object::CreateTriangleData(
	ID3D12Device4* device4)
{
	Vertex triangleVertices[4] =
	{
		-1.0f, -1.0f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		-1.0f, 1.0f, 1.0f,	//v1
		1.0f, 0.0f, 0.0f,	//v1 color

		1.0f, -1.0f, 0.0f, //v2
		1.0f, 0.0f, 0.0f,	//v2 color

		1.0f, 1.0f, -1.0f, //v3
		1.0f, 0.0f, 0.0f	//v3 color
	};

	const UINT byteWidth = sizeof(triangleVertices);
	m_resource.Initialize(
		device4,
		byteWidth,
		D3D12_HEAP_FLAG_NONE,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_FLAG_NONE);

	//Initialize vertex buffer view, used in the render call.
	vertexBufferView.BufferLocation = m_resource.mp_resource->GetGPUVirtualAddress();//vertexBufferResource->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = sizeof(triangleVertices);
}
