#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <assert.h>
#include <string>
#include <vector>
#include <cassert>  
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include <unordered_map>
#include <algorithm>
#include "MathFunc.h"

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
	float padding0[3];
	Matrix4x4 uvTransform;
	float shininess;
	float padding1[3];
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};

// 三つ組のキー
struct TripletKey {
	uint32_t v, vt, vn;
	bool operator == (const TripletKey&) const = default;
};

struct TripletHash {
	size_t operator()(const TripletKey& k) const noexcept {
		size_t h = 0;
		auto mix = [&](uint32_t x) {
			h ^= std::hash<uint32_t>{}(x)+0x9e3779b97f4a7c15ULL + (h << 5) + (h >> 2);
			};

		mix(k.v);
		mix(k.vt);
		mix(k.vn);
		return h;
	}
};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

// モデル関係の構造体
struct ModelData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	MaterialData material;
	Node rootNode;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius; // 光が届く最大距離
	float decay;
	float padding[2];
};

struct SpotLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance; // 光が届く最大距離
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float padding;
};

struct Particle {
	Transform transform;
	Vector3 rotateVelocity;
	Vector3 velocity;
	Vector4 color;
	Vector4 startColor;
	Vector4 endColor;
	Vector3 startScale;
	Vector3 endScale;
	float lifeTime;
	float currentTime;
};

enum class SpawnShape {
	Box,
	Sphere
};

struct ParticleConfig {
	Vector3 minVelocity = { -0.1f, -0.1f, -0.1f }; // 速度の最小値
	Vector3 maxVelocity = { 0.1f,  0.1f,  0.1f }; // 速度の最大値
	Vector3 minOffset = { -0.5f, -0.5f, -0.5f }; // オフセットの最小値
	Vector3 maxOffset = { 0.5f,  0.5f,  0.5f }; // オフセットの最大値
	Vector4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色
	Vector4 endColor = { 1.0f, 1.0f, 1.0f, 0.0f }; // 終了色
	Vector4 startColorMin = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色の最小値
	Vector4 startColorMax = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色の最大値
	Vector4 endColorMin = { 1.0f, 1.0f, 1.0f, 0.0f };   // 終了色の最小値
	Vector4 endColorMax = { 1.0f, 1.0f, 1.0f, 0.0f };   // 終了色の最大値
	Vector3 startScaleMin = { 0.5f, 0.5f, 0.5f }; // 初期スケールの最小値
	Vector3 startScaleMax = { 1.0f, 1.0f, 1.0f }; // 初期スケールの最大値
	Vector3 endScaleMin = { 0.5f, 0.5f, 0.5f };   // 終了スケールの最小値
	Vector3 endScaleMax = { 1.5f, 1.5f, 1.5f };   // 終了スケールの最大値
	float minLifeTime = 1.0f; // 生存時間の最小値
	float maxLifeTime = 3.0f; // 生存時間の最大値
	Vector3 minRotate = { 0.0f, 0.0f, 0.0f }; // 回転角の最小値
	Vector3 maxRotate = { 0.0f, 0.0f, 0.0f }; // 回転角の最大値
	Vector3 minRotateVelocity = { 0.0f, 0.0f, 0.0f }; // 回転速度の最小値
	Vector3 maxRotateVelocity = { 0.0f, 0.0f, 0.0f }; // 回転速度の最大値
	SpawnShape shape = SpawnShape::Box; // 範囲タイプ
	Vector3 boxMin = { -0.5f, -0.5f, -0.5f }; // 箱の最小値
	Vector3 boxMax = { 0.5f,  0.5f,  0.5f }; // 箱の最大値
	float sphereRadius = 1.0f; // 球の半径
};

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct CameraForGPU {
	Vector3 worldPosition;
	float padding;
};

static constexpr uint32_t kMaxPointLights = 512;
static constexpr uint32_t kMaxSpotLights = 512;

struct PointLightGroup {
	PointLight lights[kMaxPointLights];
	int32_t count;
	float pad[3];
};

struct SpotLightGroup {
	SpotLight lights[kMaxSpotLights];
	int32_t count;
	float pad[3];
};

struct DebugVertex {
	Vector4 position;
	Vector4 color;
};

enum class DebugDrawMode {
	Wireframe,
	Solid
};

enum class FieldSpace {
	Local,
	World
};

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
