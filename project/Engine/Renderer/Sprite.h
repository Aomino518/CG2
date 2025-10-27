#pragma once
#include <d3d12.h>
#include <cstdint>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "Graphics.h"
#include "TextureManager.h"
#include "Color.h"

class SpriteCommon;

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	uint32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

class Sprite {
public:
	void Init(SpriteCommon* spriteCommon_);

	void Update();

	void Draw();

	void SetTexture(uint32_t textureId) { 
		textureIndex_ = textureId;
		textureSrvHandleGPU_ = TextureManager::GetGPUHandle(textureId);
	}

	Transform& TransformRef() { return transform_; }
	void SetMaterial(Vector4 material) { materialData->color = material; }
	void SetUvTransform(Transform uvTransform) { uvTransform_ = uvTransform; }

	// Getter
	const Vector2& GetPosition() const { return position_; }
	float GetRotation() const { return rotation_; }
	const Vector4& GetColor() const { return materialData->color; }
	const Vector2& GetSize() const { return size_; }
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }
	bool GetFlipX() const { return isFlipX_; }
	bool GetFlipY() const { return isFlipY_; }
	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }
	const Vector2& GetTextureSize() const { return textureSize_; }

	// Setter
	void SetPosition(const Vector2& position) { this->position_ = position; }
	void SetRotation(float rotation) { this->rotation_ = rotation; }
	void SetColor(const Vector4& color_) { materialData->color = color_; }
	void SetSize(const Vector2& size) { this->size_ = size; }
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

	void Create(SpriteCommon* common, uint32_t textureId, const Vector2& pos, const Vector4& color, const Vector2& size = { 0.0f, 0.0f });
	void Move(const Vector2& delta);
	void Rotate(float deltaAngle);
	void Scale(float factor);
	void Scale(const Vector2& factor);

private:
	SpriteCommon* spriteCommon = nullptr;

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

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

	Vector2 position_ = { 0.0f, 0.0f };
	float rotation_ = 0.0f;
	Vector2 size_ = { 360.0f, 360.0f };

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
};
