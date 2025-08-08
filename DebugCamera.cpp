#include "DebugCamera.h"

void DebugCamera::Initialize()
{

}

void DebugCamera::Update()
{
	// マウスの現在地の取得
	POINT mousePos;

	GetCursorPos(&mousePos);
	ScreenToClient(GetActiveWindow(), &mousePos);

	// マウス右ボタンが押されているか
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
		if (isRightDrag_) {
			// マウスの差分から回転角計算
			float dx = static_cast<float>(mousePos.x - preMousePos_.x);
			float dy = static_cast<float>(mousePos.y - preMousePos_.y);

			rotation_.y += dx * rotateSpeed_;
			rotation_.x += dy * rotateSpeed_;
		}
		isRightDrag_ = true;
	}
	else {
		isRightDrag_ = false;
	}

	preMousePos_ = mousePos;

	Vector3 move = { 0, 0, 0 };

	if (GetAsyncKeyState('W') & 0x8000) {
		const float speed = 1.0f;

		move = { 0, 0, speed };
		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotation_.x);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotation_.y);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotation_.z);
		Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
		Vector3 worldMove = TransformNormal(move, rotateMatrix);
		translation_ = translation_ + worldMove * moveSpeed_;
	}

	if (GetAsyncKeyState('S') & 0x8000) {
		const float speed = 1.0f;

		move = { 0, 0, -speed };
		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotation_.x);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotation_.y);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotation_.z);
		Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
		Vector3 worldMove = TransformNormal(move, rotateMatrix);
		translation_ = translation_ + worldMove * moveSpeed_;
	}

	if (GetAsyncKeyState('D') & 0x8000) {
		const float speed = 1.0f;

		move = { speed, 0, 0 };
		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotation_.x);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotation_.y);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotation_.z);
		Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
		Vector3 worldMove = TransformNormal(move, rotateMatrix);
		translation_ = translation_ + worldMove * moveSpeed_;
	}

	if (GetAsyncKeyState('A') & 0x8000) {
		const float speed = 1.0f;

		move = { -speed, 0, 0 };
		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotation_.x);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotation_.y);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotation_.z);
		Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix;
		Vector3 worldMove = TransformNormal(move, rotateMatrix);
		translation_ = translation_ + worldMove * moveSpeed_;
	}

	Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1, 1, 1 }, rotation_, translation_);
	viewMatrix_ = Inverse(cameraMatrix);
}
