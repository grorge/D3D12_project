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

	 //key_W 0x57
	 //key_A 0x41
	 //key_S 0x53
	 //key_D 0x44
	 //key_Space 0x20

	this->keyBoardInt[0] = (int)(GetAsyncKeyState(0x57));
	this->keyBoardInt[1] = (int)(GetAsyncKeyState(0x41));
	this->keyBoardInt[2] = (int)(GetAsyncKeyState(0x53));
	this->keyBoardInt[3] = (int)(GetAsyncKeyState(0x44));
	this->keyBoardInt[4] = (int)(GetAsyncKeyState(0x20));
	// This can be done in threads
	for (int i = 0; i < 32; i++)
	{
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
