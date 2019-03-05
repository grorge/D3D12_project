#pragma once
#include "D3DHeader.h"
//#include "DefaultResource.h"

// include thread.h

#pragma comment (lib, "User32.lib")

class DefaultResource;

class KeyBoardInput
{
public:
	KeyBoardInput();
	~KeyBoardInput();
	void init();

	bool readKeyboard();
	void sendToGPU();
	void printKeyboard();

	int keyBoardInt[32];
	UINT keyboardSize;
private:
	PBYTE keyBoardState;
};
