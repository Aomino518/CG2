#include "DebugCamera.h"

void DebugCamera::Initialize()
{
	matRot_ = MakeIdentity4x4();
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
			float sensitivity = 0.005f;
			// マウスの差分から回転角計算
			float rotateYaw = static_cast<float>(mousePos.x - preMousePos_.x) * sensitivity;
			float rotatePitch = static_cast<float>(mousePos.y - preMousePos_.y) * sensitivity;

			Matrix4x4 matRotDelta = MakeIdentity4x4();
			matRotDelta *= MakeRotateYMatrix(rotateYaw);
			matRotDelta *= MakeRotateXMatrix(rotatePitch);
			// 累積回転行列に合成
			matRot_ = Multiply(matRotDelta, matRot_);
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
		move = TransformNormal(move, matRot_);
		translation_ = translation_ + move;
	}

	if (GetAsyncKeyState('S') & 0x8000) {
		const float speed = 1.0f;

		move = { 0, 0, -speed };
		move = TransformNormal(move, matRot_);
		translation_ = translation_ + move;
	}

	if (GetAsyncKeyState('D') & 0x8000) {
		const float speed = 1.0f;

		move = { speed, 0, 0 };
		move = TransformNormal(move, matRot_);
		translation_ = translation_ + move;
	}

	if (GetAsyncKeyState('A') & 0x8000) {
		const float speed = 1.0f;

		move = { -speed, 0, 0 };
		move = TransformNormal(move, matRot_);
		translation_ = translation_ + move;
	}

	if (GetAsyncKeyState('E') & 0x8000) {
		const float speed = 1.0f;

		move = { 0, speed, 0 };
		move = TransformNormal(move, matRot_);
		translation_ = translation_ + move;
	}

	if (GetAsyncKeyState('Q') & 0x8000) {
		const float speed = 1.0f;

		move = { 0, -speed, 0 };
		move = TransformNormal(move, matRot_);
		translation_ = translation_ + move;
	}

	// 移動行列
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

	// ワールド行列
	Matrix4x4 matWorld = Multiply(matRot_, matTrans);

	viewMatrix_ = Inverse(matWorld);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(Application::GetWidth()) / float(Application::GetHeight()), 0.1f, 100.0f);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix);
}

Matrix4x4 DebugCamera::GetBillboardMatrix() {
	Matrix4x4 invView = Inverse(viewMatrix_);

	// 平行移動は不要なので0.0fに
	invView.m[3][0] = 0.0f;
	invView.m[3][1] = 0.0f;
	invView.m[3][2] = 0.0f;

	return invView;
}
