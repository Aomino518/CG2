#pragma once
#include "Particle2DManager.h"
#include "AccelerationField2D.h"

class Particle2DEmitter
{
public:
	/// <summary>
	/// Emitterコンストラクタ
	/// </summary>
	/// <param name="groupName">名称</param>
	/// <param name="config">コンフィグ</param>
	/// <param name="count">1回にスポーンする数</param>
	/// <param name="frequency">スポーン間隔</param>
	Particle2DEmitter(const std::string& groupName,
		const ParticleConfig& config,
		uint32_t count = 10,
		float frequency = 0.1f);
	void EmitOnce();
	void StartLoop();
	void StopLoop();
	void Update();

	void DrawDebug();

	// Getter関数
	bool GetIsLoop() { return isLoop_; }
	uint32_t GetCount() const { return count_; }
	float GetFrenquency() const { return frequency_; }
	const Transform& GetTransform() const { return transform_; }
	const ParticleConfig& GetConfig() const { return config_; }
	const AccelerationField2D& GetLocalField() const { return localField_; }

	// Setter関数
	void SetCount(uint32_t count) { this->count_ = count; }
	void SetFrenquency(float frequency) { this->frequency_ = frequency; }
	void SetTransform(Transform transform) { this->transform_ = transform; }
	void SetConfig(const ParticleConfig& config) { this->config_ = config; };
	void SetSpawnShapeBox(const Vector2& min, const Vector2& max);
	void SetSpawnShapeSphere(float radius);
	void SetLocalField(AccelerationField2D& localField) { this->localField_ = localField; }

	void DrawImGui();

	// Json保存と読み込み
	json SaveToJson() const;
	void LoadFromJson(const json& j);

private:
	std::string groupName_;
	Transform transform_{};
	ParticleConfig config_{};
	uint32_t count_ = 10; // 1回の発生個数
	float frequency_ = 0.0f; // 発生頻度
	float frequencyTime_ = 0.0f; // 発生タイマー
	bool isLoop_ = false; // 無限発生か

	const float kDeltaTime = 1.0f / 60.0f;
	AccelerationField2D localField_;
};