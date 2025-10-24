#pragma once
#include <d3d12.h>
#include <cstdint>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "Graphics.h"

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

	void SetTexture(D3D12_GPU_DESCRIPTOR_HANDLE handle) { textureSrvHandleGPU_ = handle; }
	void SetTransform(Transform transform) { 
		transform_.translate = transform.translate; 
		transform_.rotate = transform.rotate;
		transform_.scale = transform.scale;
	}
	Transform& TransformRef() { return transform_; }
	void SetMaterial(Vector4 material) { materialData->color = material; }
	void SetUvTransform(Transform uvTransform) { uvTransform_ = uvTransform; }

	// Getter
	const Vector2& GetPosition() const { return position; }
	float GetRotation() const { return rotation; }
	const Vector4& GetColor() const { return materialData->color; }
	const Vector2& GetSize() const { return size; }

	// Setter
	void SetPosition(const Vector2& position_) { this->position = position_; }
	void SetRotation(float rotation_) { this->rotation = rotation_; }
	void SetColor(const Vector4& color_) { materialData->color = color_; }
	void SetSize(const Vector2& size_) { this->size = size_; }

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

	Transform uvTransform_ = {};

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

	Vector2 position = { 0.0f, 0.0f };
	float rotation = 0.0f;
	Vector2 size = { 360.0f, 360.0f };
};
