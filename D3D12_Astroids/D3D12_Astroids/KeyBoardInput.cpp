#include "KeyBoardInput.h"

KeyBoardInput::KeyBoardInput()
{
}

KeyBoardInput::~KeyBoardInput()
{
}

void KeyBoardInput::init()
{
	//Launch 256 threads
}

void KeyBoardInput::readKeyboard()
{
	GetKeyboardState(this->keyBoardState);
	bool keyPressed = false;

	// This can be done in threads
	for (int i = 0; i < 256; i++)
	{
		this->keyBoardChars[i] = (char)(GetAsyncKeyState(i));
		if (this->keyBoardChars[i] > 0)
			keyPressed = true;
	}

	if (keyPressed)
		this->printKeyboard();
}

void KeyBoardInput::sendToGPU()
{
	
}

void KeyBoardInput::printKeyboard()
{
 	// This is slow
	for (int i = 0; i < 256; i++)
	{
		printToDebug(this->keyBoardChars[i]);
		printToDebug("|");
	}
	printToDebug("\n");
}
