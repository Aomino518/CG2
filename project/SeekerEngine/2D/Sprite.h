#pragma once
#include <d3d12.h>
#include <cstdint>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Graphics.h"
#include "TextureManager.h"
#include "CreateResorceUtils.h"
#include "BlendStateUtils.h"
#include "JsonTransform.h"

class SpriteCommon;

class Sprite {
public:
	void Init();

	void Update();

	void Draw();

	void SetTexture(uint32_t textureId) { 
		textureIndex_ = textureId;
		textureSrvHandleGPU_ = TextureManager::GetInstance()->GetGPUHandle(textureId);
	}

	Transform& TransformRef() { return transform_; }
	void SetMaterial(Vector4 material) { materialData->color = material; }
	void SetUvTransform(Transform uvTransform) { uvTransform_ = uvTransform; }

	// Getter
	Vector2 GetPosition() const { return { transform_.translate.x, transform_.translate.y }; }
	float GetRotation() const { return transform_.rotate.z; }
	Vector2 GetSize() const { return { transform_.scale.x, transform_.scale.y }; }
	const Vector4& GetColor() const { return materialData->color; }
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }
	bool GetFlipX() const { return isFlipX_; }
	bool GetFlipY() const { return isFlipY_; }
	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }
	const Vector2& GetTextureSize() const { return textureSize_; }
	const Transform& GetUV() const { return uvTransform_; }
	const BlendMode& GetBlendMode() const { return mode_; }
	const Transform& GetTransform() const { return transform_; }

	// Setter
	void SetPosition(const Vector2& position) { 
		this->transform_.translate.x = position.x; 
		this->transform_.translate.y = position.y; 
	}
	void SetRotation(float rotation) { this->transform_.rotate.z = rotation; }
	void SetSize(const Vector2& size) { 
		this->transform_.scale.x = size.x; 
		this->transform_.scale.y = size.y; 
	}
	void SetColor(const Vector4& color_) { materialData->color = color_; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint_ = anchorPoint; }
	void SetFlipX(bool isFlipX) { this->isFlipX_ = isFlipX; }
	void SetFlipY(bool isFlipY) { this->isFlipY_ = isFlipY; }
	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop_ = textureLeftTop; }
	void SetTextureSize(const Vector2& textureSize) { this->textureSize_ = textureSize; }
	void SetAlpha(float a) { materialData->color.w = a; }
	void SetColorRGB(float r, float g, float b) {
		materialData->color.x = r;
		materialData->color.y = g;
		materialData->color.z = b;
	}
	void SetBlendMode(BlendMode mode);
	void SetTransform(const Transform& transform) { this->transform_ = transform; }

	/// <summary>
	/// スプライトを生成
	/// </summary>
	/// <param name="textureId">テクスチャハンドル</param>
	/// <param name="pos">スプライトの座標</param>
	/// <param name="color">スプライトの色</param>
	/// <param name="size">スプライトのサイズ</param>
	void Create(uint32_t textureId, const Vector2& pos, const Vector4& color, const Vector2& size = { 0.0f, 0.0f });
	void Move(const Vector2& delta);
	void Rotate(float deltaAngle);
	void Scale(float factor);
	void Scale(const Vector2& factor);
	/// <summary>
	/// 点滅する色の更新
	/// </summary>
	/// <param name="baseColor">元の色</param>
	/// <param name="blinkColor">点滅させる色</param>
	void UpdateColorBlink(const Vector4& baseColor, const Vector4& blinkColor);

	void DrawImGui();

	// Json保存と読み込み
	json SaveToJson() const;
	void LoadFromJson(const json& j);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	TransformationMatrix* transformationMatrixData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
	Transform transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};

	Transform uvTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// アンカーポイント
	Vector2 anchorPoint_ = { 0.0f, 0.0f };

	// 左右フリップ
	bool isFlipX_ = false;
	// 上下フリップ
	bool isFlipY_ = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f, 0.0f };
	// テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 100.0f, 100.0f };

	// テクスチャ番号
	uint32_t textureIndex_ = 0;

	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

	// ブレンドモード取得
	BlendMode mode_ = kBlendModeNone;

	// 色の点滅
	int colorBlinkFrame_ = 0;
	float colorBlinkSpeed_ = 0.1f;
};
