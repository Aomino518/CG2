#pragma once
#include "CreateResorceUtils.h"
#include <numbers>

class Camera
{
public:
	Camera();
	void Update();

	// Getter
	const Transform& GetTransform() const { return transform_; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetScale() const { return transform_.scale; }
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	Matrix4x4 GetBillboardMatrix();

	// Setter
	void SetTranslate(const Vector3& translate) { this->transform_.translate = translate; }
	void SetRotate(const Vector3& rotate) { this->transform_.rotate = rotate; }
	void SetScale(const Vector3& scale) { this->transform_.scale = scale; }
	void SetFovY(float fovY) { this->fovY_ = fovY; }
	void SetAspectRatio(float aspectRatio) { this->aspectRatio_ = aspectRatio; }
	void SetNearClip(float nearClip) { this->nearClip_ = nearClip; }
	void SetFarClip(float farClip) { this->farClip_ = farClip; }

private:
	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	float fovY_;
	float aspectRatio_;
	float nearClip_;
	float farClip_;
};

