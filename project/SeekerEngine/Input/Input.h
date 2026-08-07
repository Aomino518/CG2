#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Xinput.h>
#include <assert.h>
#include <wrl.h>
#include "Vector2.h"

class Application;
class Input
{
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	static Input* GetInstance();

	void Init();
	void Shutdown();

	void Update();

	// キーボード入力用判定
	bool IsTrigger(int dik) const;
	bool IsPress(int dik) const;
	bool IsRelease(int dik) const;
	
	// ゲームパッド用入力判定
	bool IsXbBtnTrigger(int btn) const;
	bool IsXbBtnPress(int btn) const;
	bool IsXbBtnRelease(int btn) const;

	/// <summary>
	/// 左トリガーの押し込み数値を返す関数
	/// </summary>
	/// <returns>0.0f～1.0f</returns>
	float GetXbLeftTrigger() const;
	/// <summary>
	/// 右トリガーの押し込み数値を返す関数
	/// </summary>
	/// <returns>0.0f～1.0f</returns>
	float GetXbRightTrigger() const;

	// 左スティック
	Vector2 GetXbLeftStickVector();
	float GetXbLeftStickMagnitude();

	// 右スティック
	Vector2 GetXbRightStickVector();
	float GetXbRightStickMagnitude();

	// バイブレーション
	void SetVibration(float leftMotor, float rightMotor);

private:
	Vector2 GetStickVector(float stickX, float stickY, float& normedMagnitude, int stickDeadZoon);

private:
	// 内部用
	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	ComPtr<IDirectInput8> directInput_ = nullptr;
	ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	BYTE key[256] = {};
	BYTE preKey[256] = {};
	Application* app_ = nullptr;

	// Xboxコントローラー
	XINPUT_STATE state_;
	XINPUT_STATE prevState_;

	// スティックの倒し具合
	float normedMagnitudeLS_ = 0.0f;
	float normedMagnitudeRS_ = 0.0f;

	// 振動
	XINPUT_VIBRATION vibration_;
};

