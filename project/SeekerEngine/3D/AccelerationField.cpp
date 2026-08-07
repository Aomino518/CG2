#include "AccelerationField.h"
#include "DebugDraw.h"
#include <nlohmann/json.hpp>
#include "JsonTransform.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif


void AccelerationField::DrawDebug(const Vector3& origin)
{
#ifdef _DEBUG
	AABB worldAABB = GetWorldAABB(origin);
	if(space_ == FieldSpace::Local) {
		DebugDraw::DrawAABB({ 0.0f, 0.0f, 0.0f }, worldAABB, Vector4(0.2f, 0.8f, 1.0f, 1.0f), DebugDrawMode::Wireframe);
	} else if(space_ == FieldSpace::World){
		DebugDraw::DrawAABB({ 0.0f, 0.0f, 0.0f }, worldAABB, Vector4(0.2f, 1.0f, 0.4f, 1.0f), DebugDrawMode::Wireframe);
	}
#endif
}

AABB AccelerationField::GetWorldAABB(const Vector3& origin) const
{
	AABB result = area_;

	if (space_ == FieldSpace::Local) {
		result.min = result.min + origin + position_;
		result.max = result.max + origin + position_;
	} else {
		result.min = result.min + position_;
		result.max = result.max + position_;
	}

	return result;
}

json AccelerationField::SaveToJson() const {
	return json{
		{"translate", ToJson(position_)},
		{"acceleration", ToJson(acceleration_)},
		{"areaMax", ToJson(area_.max)},
		{"areaMin", ToJson(area_.min)},
		{"isActive", isActive_}
	};
}

void AccelerationField::LoadFromJson(const json& j) {
	if (j.contains("translate")) {
		FromJson(j.at("translate"), position_);
	}

	if (j.contains("acceleration")) {
		FromJson(j.at("acceleration"), acceleration_);
	}

	if (j.contains("areaMax")) {
		FromJson(j.at("areaMax"), area_.max);
	}

	if (j.contains("areaMin")) {
		FromJson(j.at("areaMin"), area_.min);
	}

	if (j.contains("isActive")) {
		isActive_ = j.at("isActive").get<bool>();
	}
}

void AccelerationField::ImGuiDraw()
{
#ifdef USE_IMGUI
	// =========================
	// LocalField Control
	// =========================
	ImGui::DragFloat3("Acceleration", (float*)&acceleration_, 0.1f);
	ImGui::DragFloat3("Min Field", (float*)&area_.min, 0.1f);
	ImGui::DragFloat3("Max Field", (float*)&area_.max, 0.1f);
	ImGui::Checkbox("Active", &isActive_);
#endif
}
