#include "WorldField2DManager.h"
#include "Logger.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

WorldField2DManager* WorldField2DManager::GetInstance()
{
	static WorldField2DManager instance;
	return &instance;
}

void WorldField2DManager::CreateWorldField(std::string name, Vector2 position, Vector2 acceleration, AABB2D area, bool isActive)
{
	AccelerationField2D field;
	field.SetSpace(FieldSpace::World);
	field.SetPosition(position);
	field.SetAcceleration(acceleration);
	field.SetAABB2D(area);
	field.SetIsActive(isActive);
	worldFields_.emplace(name, std::move(field));
}

void WorldField2DManager::RemoveField(const std::string& name)
{
	auto it = worldFields_.find(name);
	if (it == worldFields_.end()) {
		return;
	}

	worldFields_.erase(it);

	Logger::Write("WorldField2D removed: " + name);
}

void WorldField2DManager::AllRemoveField()
{
	worldFields_.clear();
}

AccelerationField2D* WorldField2DManager::GetWorldField(const std::string& name)
{
	auto it = worldFields_.find(name);
	assert(it != worldFields_.end() && "WorldField not found");
	return &it->second;
}

std::vector<AccelerationField2D*> WorldField2DManager::GetAllWorldFields() const
{
	std::vector<AccelerationField2D*> result;
	result.reserve(worldFields_.size());
	for(const auto& worldField : worldFields_) {
		result.push_back(const_cast<AccelerationField2D*>(&worldField.second));
	}
	return result;
}

void WorldField2DManager::DrawDebug()
{
	for (auto& field : worldFields_) {
		field.second.DrawDebug(field.second.GetPosition());
	}
}

void WorldField2DManager::DrawImGui(const std::string& name)
{
#ifdef USE_IMGUI
	auto it = worldFields_.find(name);
	if (it != worldFields_.end()) {
		ImGui::Text("Name: %s", it->first.c_str());
		bool changed = false;
		Vector2 pos = it->second.GetPosition();
		Vector2 accelerarion = it->second.GetAcceleration();
		AABB2D area = it->second.GetAABB();
		bool isActive = it->second.GetIsActive();

		changed |= ImGui::DragFloat2("Position", (float*)&pos, 0.01f);
		changed |= ImGui::DragFloat2("Acceleration", (float*)&accelerarion, 0.01f);
		// minをmaxが下回ると落ちるのでのちのち修正
		changed |= ImGui::DragFloat2("AABB Min", (float*)&area.min, 0.1f);
		changed |= ImGui::DragFloat2("AABB Max", (float*)&area.max, 0.1f);
		changed |= ImGui::Checkbox("isActive", &isActive);

		if (changed) {
			it->second.SetPosition(pos);
			it->second.SetAcceleration(accelerarion);
			it->second.SetAABB2D(area);
			it->second.SetIsActive(isActive);
			SetField(name, it->second);
		}
	}
#endif
}

nlohmann::json WorldField2DManager::SaveToJson() const
{
	nlohmann::json j;

	for (auto& [name, field] : worldFields_) {
		j[name] = field.SaveToJson();
		j[name]["space"] = static_cast<int>(field.GetSpace());
	}

	return j;
}

void WorldField2DManager::LoadFromJson(const nlohmann::json& json)
{
	for (auto& [name, fieldJson] : json.items()) {
		AccelerationField2D field;
		field.LoadFromJson(fieldJson);

		if (fieldJson.contains("space")) {
			field.SetSpace(static_cast<FieldSpace>(fieldJson["space"].get<int>()));
		}

		worldFields_[name] = field;
	}
}
