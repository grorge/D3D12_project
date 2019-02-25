#pragma once
#include "D3DHeader.h"

// include thread.h

#pragma comment (lib, "User32.lib")


class KeyBoardInput
{
public:
	KeyBoardInput();
	~KeyBoardInput();
	void init();

	void readKeyboard();
	void sendToGPU();
	void printKeyboard();

private:
	PBYTE keyBoardState;
	char keyBoardChars[256];
};
