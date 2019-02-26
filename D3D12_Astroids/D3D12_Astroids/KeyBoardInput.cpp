#include "KeyBoardInput.h"

KeyBoardInput::KeyBoardInput()
{
}

KeyBoardInput::~KeyBoardInput()
{
}

//void KeyBoardInput::init(DefaultResource* p_uavResourceIntArray)
void KeyBoardInput::init()
{
	//Launch 256 threads
	this->keyboardSize = ((sizeof(int) * 256) + 255) & ~255;

	//this->p_uavResourceIntArray = p_uavResourceIntArray;
}

bool KeyBoardInput::readKeyboard()
{
	//GetKeyboardState(this->keyBoardState);
	bool keyPressed = false;

	// This can be done in threads
	for (int i = 0; i < 256; i++)
	{
		this->keyBoardInt[i] = (int)(GetAsyncKeyState(i));
		if (this->keyBoardInt[i] > 0)
			keyPressed = true;
	}

	//if (keyPressed)
		//this->printKeyboard();
	return keyPressed;
}

void KeyBoardInput::sendToGPU()
{
	printToDebug("key was recorded", 1);
	//this->UploadData(&keyBoardInt, keyboardSize, &p_uavResourceIntArray);
}

void KeyBoardInput::printKeyboard()
{
 	// This is slow
	for (int i = 0; i < 256; i++)
	{
		printToDebug(this->keyBoardInt[i]);
		printToDebug("|");
	}
	printToDebug("\n");
}
