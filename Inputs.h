#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <assert.h>


class Inputs
{
public:
	bool Init(HINSTANCE hInst, HWND hwnd);
	void Shutdown();

	void Update();
	bool IsDown(int dik) const;
	bool IsPressed(int dik) const;
	bool IsReleased(int dik) const;

private:
	IDirectInput8* directInput_ = nullptr;
	IDirectInputDevice8* keyboard_ = nullptr;
	BYTE key[256] = {};
	BYTE preKey[256] = {};
};

