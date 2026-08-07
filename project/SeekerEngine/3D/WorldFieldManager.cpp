#include "WorldFieldManager.h"
#include "Logger.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

WorldFieldManager* WorldFieldManager::GetInstance() {
	static WorldFieldManager instance;
	return &instance;
}

void WorldFieldManager::CreateWorldField(std::string name, Vector3 position, Vector3 acceleration, AABB area, bool isActive)
{
	AccelerationField field;
	field.SetSpace(FieldSpace::World);
	field.SetPosition(position);
	field.SetAcceleration(acceleration);
	field.SetAABB(area);
	field.SetIsActive(isActive);
	worldFields_.emplace(name, std::move(field));
}

void WorldFieldManager::RemoveField(const std::string& name)
{
	auto it = worldFields_.find(name);
	if (it == worldFields_.end()) {
		return;
	}

	worldFields_.erase(it);

	Logger::Write("WorldField removed: " + name);
}

void WorldFieldManager::AllRemoveField()
{
	worldFields_.clear();
}

AccelerationField* WorldFieldManager::GetWorldField(const std::string& name)
{
	auto it = worldFields_.find(name);
	assert(it != worldFields_.end() && "WorldField not found");
	return &it->second;
}

std::vector<AccelerationField*> WorldFieldManager::GetAllWorldFields() const
{
	std::vector<AccelerationField*> result;
	result.reserve(worldFields_.size());
	for (const auto& worldfield : worldFields_) {
		result.push_back(const_cast<AccelerationField*>(&worldfield.second));
	}
	return result;
}

void WorldFieldManager::DrawDebug()
{
	for (auto& field : worldFields_) {
		field.second.DrawDebug(field.second.GetPosition());
	}
}

void WorldFieldManager::DrawImGui(const std::string& name)
{
#ifdef USE_IMGUI
	auto it = worldFields_.find(name);
	if (it != worldFields_.end()) {
		ImGui::Text("Name: %s", it->first.c_str());
		bool changed = false;
		Vector3 pos = it->second.GetPosition();
		Vector3 accelerarion = it->second.GetAcceleration();
		AABB area = it->second.GetAABB();
		bool isActive = it->second.GetIsActive();

		changed |= ImGui::DragFloat3("Position", (float*)&pos, 0.01f);
		changed |= ImGui::DragFloat3("Acceleration", (float*)&accelerarion, 0.01f);
		// minをmaxが下回ると落ちるのでのちのち修正
		changed |= ImGui::DragFloat3("AABB Min", (float*)&area.min, 0.1f);
		changed |= ImGui::DragFloat3("AABB Max", (float*)&area.max, 0.1f);
		changed |= ImGui::Checkbox("isActive", &isActive);

		if (changed) {
			it->second.SetPosition(pos);
			it->second.SetAcceleration(accelerarion);
			it->second.SetAABB(area);
			it->second.SetIsActive(isActive);
			SetField(name, it->second);
		}
	}
#endif
}

nlohmann::json WorldFieldManager::SaveToJson() const
{
	nlohmann::json j;

	for (auto& [name, field] : worldFields_) {
		j[name] = field.SaveToJson();
		j[name]["space"] = static_cast<int>(field.GetSpace());
	}

	return j;
}

void WorldFieldManager::LoadFromJson(const nlohmann::json& json)
{
	for (auto& [name, fieldJson] : json.items()) {
		AccelerationField field;
		field.LoadFromJson(fieldJson);

		if (fieldJson.contains("space")) {
			field.SetSpace(static_cast<FieldSpace>(fieldJson["space"].get<int>()));
		}

		worldFields_[name] = field;
	}
}
