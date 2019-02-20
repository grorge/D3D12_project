#pragma once

#include "D3DHeader.h"
#include "Resource.h"

void CopyResource(
	Resource* pDest, 
	Resource* pSrc,
	ID3D12GraphicsCommandList* pCmdList)
{
	/*

		if pSrc is an uploadResource, transitions may not be allowed
		https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_heap_type

	*/

	D3D12_RESOURCE_BARRIER open[2];
	open[0].Type = open[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	open[0].Flags = open[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	open[0].Transition.pResource = pSrc->mp_resource;
	open[0].Transition.StateBefore = pSrc->m_currentState;
	open[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	open[0].Transition.Subresource = 0;

	open[1].Transition.pResource = pDest->mp_resource;
	open[1].Transition.StateBefore = pDest->m_currentState;
	open[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	open[1].Transition.Subresource = 0;


	D3D12_RESOURCE_BARRIER close[2];
	close[0].Type = open[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	close[0].Flags = open[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	close[0].Transition.pResource = pSrc->mp_resource;
	close[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	close[0].Transition.StateAfter = pSrc->m_currentState;
	close[0].Transition.Subresource = 0;

	close[1].Transition.pResource = pDest->mp_resource;
	close[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	close[1].Transition.StateAfter = pDest->m_currentState;
	close[1].Transition.Subresource = 0;


	pCmdList->ResourceBarrier(2, open);
	pCmdList->CopyResource(pDest->mp_resource, pSrc->mp_resource);
	pCmdList->ResourceBarrier(2, close);
}