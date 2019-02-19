#pragma once
#include "D3DHeader.h"

struct Vertex
{
	float x, y, z; // Position
	float r, g, b; // Color
};

class Object
{
public:
	Object(ID3D12Device4* device4);
	~Object();

	D3D12_VERTEX_BUFFER_VIEW GETVertexView() { return this->vertexBufferView; };
	ConstantBuffer GETConstBufferData() { return this->constantBufferCPU; };

	void addToCommList(ID3D12GraphicsCommandList3*	commandList4);
	void update(UINT bbIndex);

private:
	void CreateTriangleData(ID3D12Device4* device4);
	void CreateConstantBufferResources();

	ID3D12Device4*				device4Reff = nullptr;
	ID3D12Resource1*			vertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	vertexBufferView = {};

	ConstantBuffer				constantBufferCPU = {};
};
