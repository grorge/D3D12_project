#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>
#include <string>
#include "d3dx12.h"
#include <iostream>
#include <cstdlib>
#include <ctime>


#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

struct ConstantBuffer
{
	float values[4];
};
struct TranslatonBuffer
{
	float trans[3]; // Since we only have 2D, z will be speed
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
}inline void printToDebug(const float number, ...)
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
inline void printToDebug(const char* textToAdd, const float number, ...)
{
	std::string strText(textToAdd);
	std::string strNumb(std::to_string(number));


	std::string str = strText + strNumb + "\n";


	const char* text = str.c_str();
	char szBuff[1024];
	va_list arg;
	va_start(arg, text);
	_vsnprintf_s(szBuff, sizeof(szBuff), text, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}
inline void printToDebug(const char* textToAdd, const int number, ...)
{
	std::string strText(textToAdd);
	std::string strNumb(std::to_string(number));


	std::string str = strText + strNumb + "\n";


	const char* text = str.c_str();
	char szBuff[1024];
	va_list arg;
	va_start(arg, text);
	_vsnprintf_s(szBuff, sizeof(szBuff), text, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}
inline void printToDebug(const char* textToAdd, const int number, const char* textToAdd2, const int number2, ...)
{
	std::string strText(textToAdd);
	std::string strNumb(std::to_string(number));
	std::string strText2(textToAdd2);
	std::string strNumb2(std::to_string(number2));


	std::string str = strText + strNumb + strText2 + strNumb2 + "\n";


	const char* text = str.c_str();
	char szBuff[1024];
	va_list arg;
	va_start(arg, text);
	_vsnprintf_s(szBuff, sizeof(szBuff), text, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}