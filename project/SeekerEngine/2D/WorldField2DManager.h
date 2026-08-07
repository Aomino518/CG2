#pragma once
#include "AccelerationField2D.h"
#include <nlohmann/json.hpp>

class WorldField2DManager
{
public:
	static WorldField2DManager* GetInstance();

	void CreateWorldField(
		std::string name,
		Vector2 position = { 0.0f, 0.0f },
		Vector2 acceleration = { 0.0f, 0.0f },
		AABB2D area = { {-1.0f, -1.0f }, {1.0f, 1.0f } },
		bool isActive = true);

	/// <summary>
	/// ワールドフィールド削除関数
	/// </summary>
	/// <param name="name"></param>
	void RemoveField(const std::string& name);

	void AllRemoveField();

	// Getter関数
	AccelerationField2D* GetWorldField(const std::string& name);
	std::vector<AccelerationField2D*> GetAllWorldFields() const;
	const std::unordered_map<std::string, AccelerationField2D>& GetWorldFieldMap() const { return worldFields_; }

	// Setter関数
	void SetField(std::string name, AccelerationField2D field) { this->worldFields_[name] = field; }

	void DrawDebug();
	void DrawImGui(const std::string& name);

	// json保存と読み込み
	nlohmann::json SaveToJson() const;
	void LoadFromJson(const nlohmann::json& json);

private:
	// メンバ関数
	WorldField2DManager() = default;
	~WorldField2DManager() = default;
	WorldField2DManager(const WorldField2DManager&) = delete;
	WorldField2DManager& operator=(const WorldField2DManager&) = delete;

	// メンバ変数
	std::unordered_map<std::string, AccelerationField2D> worldFields_;
};

