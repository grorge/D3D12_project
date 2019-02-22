#pragma once
#include "D3DHeader.h"

#include "UploadResource.h"
#include "DefaultResource.h"

struct Vertex
{
	float x, y, z; // Position
	float r, g, b; // Color
};

class Object
{
public:
	Object(ID3D12Device4* device4, int test);
	~Object();

	D3D12_VERTEX_BUFFER_VIEW GETVertexView() { return this->vertexBufferView; };
	ConstantBuffer GETColorBufferData() { return this->color; };
	ConstantBuffer GETTranslationBufferData() { return this->translation; };

	void addToCommList(ID3D12GraphicsCommandList3*	commandList4);
	void update();

	DefaultResource m_resource;

private:
	void CreateTriangleData(ID3D12Device4* device4);

	ID3D12Resource1*			vertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	vertexBufferView = {};

	ConstantBuffer				color = {};
	ConstantBuffer				translation = {};

};
