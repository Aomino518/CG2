#include "Input.h"
#include "Application.h"
#include "Logger.h"
#include <cmath>
#include <Xinput.h>
#include <algorithm>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Xinput.lib")

Input* Input::GetInstance()
{
	static Input instance;
	return &instance;
}

void Input::Init()
{
	app_ = Application::GetInstance();
	//========================================
	// DirectInputの初期化
	//========================================
	HRESULT result = DirectInput8Create(app_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(
		app_->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	DWORD dwResult;
	ZeroMemory(&state_, sizeof(XINPUT_STATE));
	ZeroMemory(&vibration_, sizeof(XINPUT_VIBRATION));

	dwResult = XInputGetState(0, &state_);

	if (dwResult == ERROR_SUCCESS) {
		Logger::Write("Controller 0 is connected");
	} else {
		Logger::Write("Controller 0 is not found");
	}
}

void Input::Shutdown()
{
	ZeroMemory(&vibration_, sizeof(XINPUT_VIBRATION));
	XInputSetState(0, &vibration_);

	if (keyboard_) {
		keyboard_->Unacquire();
	}
	Logger::Write("InputManager Shutdown");
}

void Input::Update()
{
	memcpy(preKey, key, sizeof(key));
	memcpy(&prevState_, &state_, sizeof(state_));
	XInputGetState(0, &state_);

	// キーボード情報の取得開始
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key), key);
}

//==============================================
// キー入力判定
//==============================================
bool Input::IsTrigger(int dik) const
{
	return (key[dik] & 0x80) && !(preKey[dik] & 0x80);
}

bool Input::IsPress(int dik) const
{
	return (key[dik] & 0x80) != 0;
}

bool Input::IsRelease(int dik) const
{
	return !(key[dik] & 0x80) && (preKey[dik] & 0x80);
}

//==============================================
// ゲームパッド入力判定
//==============================================
bool Input::IsXbBtnTrigger(int btn) const
{
	return (state_.Gamepad.wButtons & btn) && !(prevState_.Gamepad.wButtons & btn);
}

bool Input::IsXbBtnPress(int btn) const
{
	return state_.Gamepad.wButtons & btn;
}

bool Input::IsXbBtnRelease(int btn) const
{
	return !(state_.Gamepad.wButtons & btn) && (prevState_.Gamepad.wButtons & btn);
}

float Input::GetXbLeftTrigger() const
{
	BYTE leftTrigger = state_.Gamepad.bLeftTrigger;

	if (leftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		return 0.0f;
	}

	leftTrigger -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

	// 正規化(0.0f～1.0f)
	float normLT = leftTrigger / (255.0f - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

	return normLT;
}

float Input::GetXbRightTrigger() const
{
	BYTE rightTrigger = state_.Gamepad.bRightTrigger;

	if (rightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		return 0.0f;
	}

	rightTrigger -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

	// 正規化(0.0f～1.0f)
	float normRT = rightTrigger / (255.0f - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

	return normRT;
}

Vector2 Input::GetXbLeftStickVector()
{
	return GetStickVector(
		state_.Gamepad.sThumbLX,
		state_.Gamepad.sThumbLY,
		normedMagnitudeLS_,
		XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
}

float Input::GetXbLeftStickMagnitude()
{
	return normedMagnitudeLS_;
}

Vector2 Input::GetXbRightStickVector()
{
	return GetStickVector(
		state_.Gamepad.sThumbRX,
		state_.Gamepad.sThumbRY,
		normedMagnitudeRS_,
		XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
}

float Input::GetXbRightStickMagnitude()
{
	return normedMagnitudeRS_;
}

void Input::SetVibration(float leftMotor, float rightMotor)
{
	leftMotor = std::clamp(leftMotor, 0.0f, 1.0f);
	rightMotor = std::clamp(rightMotor, 0.0f, 1.0f);

	vibration_.wLeftMotorSpeed = static_cast<WORD>(leftMotor * 65535);
	vibration_.wRightMotorSpeed = static_cast<WORD>(rightMotor * 65535);

	XInputSetState(0, &vibration_);
}

Vector2 Input::GetStickVector(float stickX, float stickY, float& normedMagnitude, int stickDeadZoon)
{
	float sx = stickX;
	float sy = stickY;

	// コントローラーをどれだけ押されてるか判断
	float magnitude = sqrt(sx * sx + sy * sy);

	if (magnitude < 0.0001f) {
        normedMagnitude = 0.0f;
        return Vector2{ 0.0f, 0.0f };
    }

	// 押された方向判定
	float normX = sx / magnitude;
	float normY = sy / magnitude;

	// スティックが円形のデッドゾーンの外か
	if (magnitude > stickDeadZoon) {
		// 入力の強さを、想定される最大値にクランプする
		if (magnitude > 32767.0f) {
			magnitude = 32767.0f;
		}

		// デッドゾーンの境界を起点とするように、入力範囲を調整
		magnitude -= stickDeadZoon;

		// 入力の強さを想定範囲に基づいて正規化
		normedMagnitude = magnitude / (32767.0f - stickDeadZoon);
	} else {
		magnitude = 0.0f;
		normedMagnitude = 0.0f;
	}

	return Vector2{ normX * normedMagnitude, normY * normedMagnitude };
}
