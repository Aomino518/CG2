#pragma once
#include "AccelerationField.h"
#include <nlohmann/json.hpp>

class WorldFieldManager
{
public:
	static WorldFieldManager* GetInstance();

	void CreateWorldField(
		std::string name, 
		Vector3 position = { 0.0f, 0.0f, 0.0f} ,
		Vector3 acceleration = { 0.0f, 0.0f, 0.0f },
		AABB area = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} },
		bool isActive = true);

	/// <summary>
	/// ワールドフィールド削除関数
	/// </summary>
	/// <param name="name"></param>
	void RemoveField(const std::string& name);

	void AllRemoveField();

	// Getter関数
	AccelerationField* GetWorldField(const std::string& name);
	std::vector<AccelerationField*> GetAllWorldFields() const;
	const std::unordered_map<std::string, AccelerationField>& GetWorldFieldMap() const { return worldFields_; }

	// Setter関数
	void SetField(std::string name, AccelerationField field) { this->worldFields_[name] = field; }

	void DrawDebug();
	void DrawImGui(const std::string& name);

	// json保存と読み込み
	nlohmann::json SaveToJson() const;
	void LoadFromJson(const nlohmann::json& json);

private:
	// メンバ関数
	WorldFieldManager() = default;
	~WorldFieldManager() = default;
	WorldFieldManager(const WorldFieldManager&) = delete;
	WorldFieldManager& operator=(const WorldFieldManager&) = delete;

	// メンバ変数
	std::unordered_map<std::string, AccelerationField> worldFields_;
};