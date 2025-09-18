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
	//Vector3 rotation_ = { 0, 0, 0 };
	// 累積回転行列
	Matrix4x4 matRot_;
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

	// 追加
	//Vector3 pivot = { 0, 0, 0 }; // 回転の中心点
	//float distance_ = 50.0f; // pivot からの距離

	//float yaw = 0.0f; // 水平回転 (Y軸)
	//float pitch_ = 0.0f; // 縦回転 (X軸)

};

