#pragma once
#include "Matrix.h"
#include <windows.h>

class DebugCamera
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	Matrix4x4 GetViewMatrix() { return viewMatrix_; }

private:
	// X,Y,Z軸回りのローカル回転角
	Vector3 rotation_ = { 0, 0, 0 };
	// ローカル座標
	Vector3 translation_ = { 0, 0, -50 };
	// ビュー行列
	Matrix4x4 viewMatrix_;
	// 射影行列
	Matrix4x4 projectionMatrix_;

	float moveSpeed_ = 0.5f;
	float rotateSpeed_ = 0.01f;

	POINT preMousePos_ = {};
	bool isRightDrag_ = false;
};

