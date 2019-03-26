#include "Renderer.h"

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h> 

HWND				InitWindow(HINSTANCE hInstance);	//1. Create Window
LRESULT CALLBACK	WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void EnableDebugLayer();

#pragma region wwinMain
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	EnableDebugLayer();

	MSG msg			= {0};
	HWND wndHandle	= InitWindow(hInstance);			//1. Create Window
	
	Renderer* render = new Renderer();
	render->init(wndHandle);
	render->startGame();

	if(wndHandle)
	{
		ShowWindow(wndHandle, nCmdShow);
		
		if (!RUN_SEQUENTIAL)
			render->initThreads();
		else
			render->running = true;
		
		
		while(WM_QUIT != msg.message && render->running)
		{
			if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if (!RUN_SEQUENTIAL)
					render->tm_main();
				else
					render->sequentialFrame();
			}
		}

		if (!RUN_SEQUENTIAL)
			render->joinThreads();

	}
	
	delete render;

	return (int)msg.wParam;
}
#pragma endregion

void EnableDebugLayer()
{
#ifdef _DEBUG
	ID3D12Debug* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease(&debugController);
#endif // _DEBUG

	
}

#pragma region InitWindow
HWND InitWindow(HINSTANCE hInstance) //1. Create Window
{
	WNDCLASSEX wcex		= {0};
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= "D3D12 Project";
	if (!RegisterClassEx(&wcex))
	{
		return false;
	}

	RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		"D3D12 Project",
		"D3D12 Project",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
}
#pragma endregion

#pragma region WndProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
#pragma endregion
