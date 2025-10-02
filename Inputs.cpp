#include "Inputs.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

bool Inputs::Init(HINSTANCE hInst, HWND hwnd)
{
	//========================================
	// DirectInputの初期化
	//========================================
	HRESULT result = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	return true;
}

void Inputs::Shutdown()
{
	if (keyboard_) {
		keyboard_->Unacquire();
		keyboard_->Release();
		keyboard_ = nullptr;
	}

	if (directInput_) {
		directInput_->Release();
		directInput_ = nullptr;
	}
}

void Inputs::Update()
{
	memcpy(preKey, key, sizeof(key));

	// キーボード情報の取得開始
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key), key);
}

bool Inputs::IsDown(int dik) const
{
	return (key[dik] & 0x80) != 0;
}

bool Inputs::IsPressed(int dik) const
{
	return (key[dik] & 0x80) && !(preKey[dik] & 0x80);
}

bool Inputs::IsReleased(int dik) const
{
	return !(key[dik] & 0x80) && (preKey[dik] & 0x80);
}
