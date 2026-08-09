#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <cassert>  
#include "MathFunc.h"
#include <unordered_map>
#include <algorithm>
#include "CreateResorceUtils.h"
#include "Model.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "CameraManager.h"
#include "BlendStateUtils.h"
#include "LightManager.h"
#include <nlohmann/json.hpp>
#include "JsonTransform.h"

class Entity3DCommon;
class TextureManager;

class Entity3D
{
public:
	// 初期化
	void Init();

	// 更新
	void Update();

	// 描画
	void Draw();

	// Json保存と読み込み
	json SaveToJson() const ;
	void LoadFromJson(const json& j);

	// getter関数
	const Transform& GetTransform() const { return transform_; }
	const Vector3& GetScale() const { return transform_.scale; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Vector4& GetMaterial() const { return model_->GetMaterial(); }
	const bool GetIsLighting() const { return model_->GetIsLighting(); }
	const BlendMode& GetBlendMode() { return mode_; }
	const Vector3& GetLightDirection() const { return directionalLightData_->direction; }
	const Vector3& GetPointLightPos() const { return pointLightData_->position; }

	// setter関数
	void SetModel(const std::string& filePath);
	void SetScale(const Vector3& scale) { this->transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { this->transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { this->transform_.translate = translate; }
	void SetCamera(Camera* camera) { this->camera_ = camera; }
	void SetMaterial(const Vector4& material) { model_->SetMaterial(material); }
	void SetBlendMode(BlendMode mode);
	void SetLightDirection(const Vector3& pos) { this->directionalLightData_->direction = pos; }
	void SetPointLightPos(const Vector3& pos) { this->pointLightData_->position = pos; }
	void SetTransform(const Transform& transform) { this->transform_ = transform; }

	void DrawImGui();

private:
	void ModelResourcesSetting();

	Transform transform_;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_ = nullptr;
	PointLight* pointLightData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource = nullptr;
	CameraForGPU* cameraData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	CameraManager* cameraManager_ = nullptr;

	// ブレンドモード取得
	BlendMode mode_ = kBlendModeNone;
};

