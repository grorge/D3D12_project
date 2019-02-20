#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

struct ConstantBuffer
{
	float values[4];
};

template<class Interface>
inline void SafeRelease(
	Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}