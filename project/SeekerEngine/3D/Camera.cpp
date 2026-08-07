#include "Camera.h"
#include "Application.h"
#include "Graphics.h"
#include "JsonTransform.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

Camera::Camera()
	: transform_({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }),
	fovY_(0.45f),
	aspectRatio_(float(Application::GetInstance()->GetWidth()) / float(Application::GetInstance()->GetHeight())),
	nearClip_(0.1f),
	farClip_(100.0f),
	worldMatrix_(MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate)),
	viewMatrix_(Inverse(worldMatrix_)),
	projectionMatrix_(MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_)),
	viewProjectionMatrix_(Multiply(viewMatrix_, projectionMatrix_))
{}

void Camera::Update()
{
	aspectRatio_ = float(Graphics::GetInstance()->GetWidth()) / float(Graphics::GetHeight());
	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

Matrix4x4 Camera::GetBillboardMatrix()
{
	Matrix4x4 invView = Inverse(viewMatrix_);

	// 平行移動は不要なので0.0fに
	invView.m[3][0] = 0.0f;
	invView.m[3][1] = 0.0f;
	invView.m[3][2] = 0.0f;

	return invView;
}

nlohmann::json Camera::SaveToJson() const
{
	return nlohmann::json {
		{"transform", TransformToJson(transform_) },
	};
}

void Camera::LoadFromJson(const nlohmann::json& j)
{
	if (j.contains("transform")) {
		TransformFromJson(j.at("transform"), transform_);
	}
}
