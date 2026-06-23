#pragma once

class InputListener
{
public:
	InputListener()
	{

	}
	~InputListener()
	{

	}

	//Callback Functions
	virtual void onKeyDown(int key) = 0;
	virtual void onKeyUp(int key) = 0;
};