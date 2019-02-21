#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>
#include <string>


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

inline void printToDebug(const char* text, ...)
{
	char szBuff[1024];
	va_list arg;
	va_start(arg, text);
	_vsnprintf_s(szBuff, sizeof(szBuff), text, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}
inline void printToDebug(const int number, ...)
{
	std::string str(std::to_string(number));

	//str = std::to_string(number);

	const char* text = str.c_str();
	char szBuff[1024];
	va_list arg;
	va_start(arg, text);
	_vsnprintf_s(szBuff, sizeof(szBuff), text, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}