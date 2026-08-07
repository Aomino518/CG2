#include "Editor.h"
#include "Sprite.h"
#include "Entity3D.h"
#include "EmitterManager.h"
#include "Emitter2DManager.h"
#include "ParticleManager.h"
#include "Particle2DManager.h"
#include "ModelManager.h"
#include "Logger.h"
#include "ImGuiManager.h"
#include "WorldFieldManager.h"
#include "WorldField2DManager.h"
#include "CameraManager.h"
#include <nlohmann/json.hpp>
#ifdef USE_IMGUI
#include "imgui.h"
#endif

Editor* Editor::GetInstance()
{
	static Editor instance;
	return &instance;
}

void Editor::Draw()
{
#ifdef USE_IMGUI
	DrawHierarchy();
	DrawInspector();
#endif
}

void Editor::RegisterSprite(const std::string& name, Sprite* sprite)
{
	sprites_[name] = sprite;
}

void Editor::RegisterModel(const std::string& name, Entity3D* model)
{
	models_[name] = model;
}

void Editor::RegisterParticle(const std::string& name)
{
	particles_[name] = EmitterManager::GetInstance()->GetEmitter(name);
}

void Editor::RegisterParticle2D(const std::string& name)
{
	particles2D_[name] = Emitter2DManager::GetInstance()->GetEmitter(name);
}

void Editor::SaveSceneJson(const std::string& path) const
{
	json root;
	root["sprites"] = json::object();
	root["models"] = json::object();
	root["particles2D"] = json::object();
	root["particles"] = json::object();
	root["isLighting"] = ModelManager::GetInstance()->GetIsModelLighting();
	root["worldField2D"] = json::object();
	root["worldField"] = json::object();
	root["cameras"] = json::array();

	for (const auto& [name, sprite] : sprites_) {
		if (!sprite) {
			continue;
		}
		root["sprites"][name] = sprite->SaveToJson();
	}

	for (const auto& [name, model] : models_) {
		if (!model) {
			continue;
		}

		root["models"][name] = model->SaveToJson();
	}

	for (const auto& [name, particle2D] : particles2D_) {
		if (!particle2D) {
			continue;
		}

		json particle2DJson = particle2D->SaveToJson();
		json managerJson = Particle2DManager::GetInstance()->SaveToJson(name);

		particle2DJson["blendMode"] = managerJson["blendMode"];
		
		root["particles2D"][name] = particle2DJson;
	}

	for (const auto& [name, particle] : particles_) {
		if (!particle) {
			continue;
		}

		json particleJson = particle->SaveToJson();
		json managerJson = ParticleManager::GetInstance()->SaveToJson(name);

		particleJson["blendMode"] = managerJson["blendMode"];
		particleJson["billboard"] = managerJson["billboard"];

		root["particles"][name] = particleJson;
	}

	root["lights"] = LightManager::GetInstance()->SaveToJson();

	root["windowState"] = ImGuiManager::GetInstance()->SaveEditorJson();

	root["worldField2D"] = WorldField2DManager::GetInstance()->SaveToJson();

	root["worldField"] = WorldFieldManager::GetInstance()->SaveToJson();

	root["cameras"] = CameraManager::GetInstance()->SaveToJson();

	std::ofstream ofs(path);
	if (!ofs.is_open()) {
		Logger::Write(Logger::LogLevel::Error, "Failed to open file" + path);
		return;
	}

	ofs << std::setw(4) << root << std::endl;
}

void Editor::LoadSceneJson(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		Logger::Write(Logger::LogLevel::Error, "Failed to open file" + path);
		return;
	}

	json root;
	try {
		ifs >> root;
	} catch (const nlohmann::json::parse_error& e) {
		Logger::Write(Logger::LogLevel::Warning, "Failed to parse json. Keep current scene values: " + std::string(e.what()));
		return;
	}

	if (root.is_null() || root.empty()) {
		Logger::Write(Logger::LogLevel::Warning, "Json is empty. Keep current scene values.");
		return;
	}
	
	if (root.contains("isLighting") && root["isLighting"].is_boolean()) {
		ModelManager::GetInstance()->SetIsLighting(root["isLighting"].get<bool>());
	}

	if (root.contains("sprites")) {
		for (auto& [name, data] : root["sprites"].items()) {
			auto it = sprites_.find(name);
			if (it != sprites_.end() && it->second) {
				it->second->LoadFromJson(data);
			}
		}
	}

	if (root.contains("models")) {
		for (auto& [name, data] : root["models"].items()) {
			auto it = models_.find(name);
			if (it != models_.end() && it->second) {
				it->second->LoadFromJson(data);
			}
		}
	}

	if (root.contains("particles2D")) {
		for (auto& [name, data] : root["particles2D"].items()) {
			auto it = particles2D_.find(name);
			if (it != particles2D_.end() && it->second) {
				it->second->LoadFromJson(data);
				Particle2DManager::GetInstance()->LoadFromJson(data, name);
			}
		}
	}

	if (root.contains("particles")) {
		for (auto& [name, data] : root["particles"].items()) {
			auto it = particles_.find(name);
			if (it != particles_.end() && it->second) {
				it->second->LoadFromJson(data);
				ParticleManager::GetInstance()->LoadFromJson(data, name);
			}
		}
	}

	if (root.contains("lights")) {
		LightManager::GetInstance()->LoadFromJson(root["lights"]);
	}

	if (root.contains("windowState")) {
		ImGuiManager::GetInstance()->LoadEditorJson(root["windowState"]);
	}

	if (root.contains("worldField2D")) {
		WorldField2DManager::GetInstance()->LoadFromJson(root["worldField2D"]);
	}

	if (root.contains("worldField")) {
		WorldFieldManager::GetInstance()->LoadFromJson(root["worldField"]);
	}

	if (root.contains("cameras")) {
		CameraManager::GetInstance()->LoadFromJson(root["cameras"]);
	}

}

void Editor::ClearSceneJson(const std::string& path)
{
	nlohmann::json json;
	json["isLighting"] = nlohmann::json::array();
	json["sprites"] = nlohmann::json::array();
	json["models"] = nlohmann::json::array();
	json["particles"] = nlohmann::json::array();
	json["particles2D"] = nlohmann::json::array();
	json["worldField"] = nlohmann::json::array();
	json["worldField2D"] = nlohmann::json::array();
	json["lights"] = nlohmann::json::array();
	json["cameras"] = nlohmann::json::array();

	std::ofstream file(path);
	file << json.dump(4);
}

void Editor::Clear()
{
	sprites_.clear();
	models_.clear();
	particles_.clear();
	particles2D_.clear();
	WorldFieldManager::GetInstance()->AllRemoveField();
	WorldField2DManager::GetInstance()->AllRemoveField();
	selection_ = {};
}

void Editor::DrawHierarchy()
{
#ifdef USE_IMGUI
	ImGui::Begin("Hierarchy");
	// ライティングのオンオフ
	bool isLighting = ModelManager::GetInstance()->GetIsModelLighting();
	bool isChangedLighting = false;

	isChangedLighting |= ImGui::Checkbox("isLighting", &isLighting);
	if (isChangedLighting) {
		ModelManager::GetInstance()->SetIsLighting(isLighting);
	}

	// =========================
	// Sprite
	// =========================
	if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& [name, sprite] : sprites_) {
			bool selected = (selection_.category == InspectorCategory::Sprite && selection_.name == name);
			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::Sprite;
				selection_.name = name;
			}
		}
	}

	// =========================
	// Model
	// =========================
	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& [name, model] : models_) {
			bool selected = (selection_.category == InspectorCategory::Model && selection_.name == name);
			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::Model;
				selection_.name = name;
			}
		}
	}

	// =========================
	// Particle2D
	// =========================
	if (ImGui::CollapsingHeader("Particle2D", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& emitters = Emitter2DManager::GetInstance()->GetEmitterMap();

		for (auto& [name, emitter] : emitters) {

			bool selected = (selection_.category == InspectorCategory::Particle2D && selection_.name == name);

			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::Particle2D;
				selection_.name = name;
			}
		}
	}

	// =========================
	// Particle
	// =========================
	if (ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& emitters = EmitterManager::GetInstance()->GetEmitterMap();

		for (auto& [name, emitter] : emitters) {

			bool selected = (selection_.category == InspectorCategory::Particle && selection_.name == name);

			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::Particle;
				selection_.name = name;
			}
		}
	}

	if (ImGui::CollapsingHeader("DirectionalLight", ImGuiTreeNodeFlags_DefaultOpen)) {
		bool selected = (selection_.category == InspectorCategory::DirectionalLight && selection_.name == "DirectionalLight");
		ImGui::PushID("DirectionalLightItem");
		if (ImGui::Selectable("DirectionalLight", selected)) {
			selection_.category = InspectorCategory::DirectionalLight;
			selection_.name = "DirectionalLight";
		}
		ImGui::PopID();
	}

	// =========================
	// PointLight
	// =========================
	if (ImGui::CollapsingHeader("PointLight", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& lights = LightManager::GetInstance()->GetPointLights();
		for (auto& [name, light] : lights) {
			bool selected = (selection_.category == InspectorCategory::PointLight && selection_.name == name);
			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::PointLight;
				selection_.name = name;
			}
		}
	}

	// =========================
	// SpotLight
	// =========================
	if (ImGui::CollapsingHeader("SpotLight", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& lights = LightManager::GetInstance()->GetSpotLights();
		for (auto& [name, light] : lights) {
			bool selected = (selection_.category == InspectorCategory::SpotLight && selection_.name == name);
			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::SpotLight;
				selection_.name = name;
			}
		}
	}

	// =========================
	// WorldField
	// =========================
	if (ImGui::CollapsingHeader("WorldField2D", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& fields = WorldField2DManager::GetInstance()->GetWorldFieldMap();
		for (auto& [name, field] : fields) {
			bool selected = (selection_.category == InspectorCategory::WorldField2D && selection_.name == name);
			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::WorldField2D;
				selection_.name = name;
			}
		}
	}

	// =========================
	// WorldField
	// =========================
	if (ImGui::CollapsingHeader("WorldField", ImGuiTreeNodeFlags_DefaultOpen)) {
		auto& fields = WorldFieldManager::GetInstance()->GetWorldFieldMap();
		for (auto& [name, field] : fields) {
			bool selected = (selection_.category == InspectorCategory::WorldField && selection_.name == name);
			if (ImGui::Selectable(name.c_str(), selected)) {
				selection_.category = InspectorCategory::WorldField;
				selection_.name = name;
			}
		}
	}

	ImGui::End();
#endif
}

void Editor::DrawInspector()
{
#ifdef USE_IMGUI
	ImGui::Begin("Inspector");

	switch (selection_.category) {

	case InspectorCategory::Sprite:
	{
		auto it = sprites_.find(selection_.name);
		if (it != sprites_.end()) {
			ImGui::Text("Name: %s", it->first.c_str());
			it->second->DrawImGui();
		}
		break;
	}

	case InspectorCategory::Model:
	{
		auto it = models_.find(selection_.name);
		if (it != models_.end()) {
			ImGui::Text("Name: %s", it->first.c_str());
			it->second->DrawImGui();
		}
		break;
	}

	case InspectorCategory::Particle2D:
	{
		auto* emitter = Emitter2DManager::GetInstance()->GetEmitter(selection_.name);

		if (emitter) {
			emitter->DrawImGui();
		}

		Particle2DManager::GetInstance()->DrawParticleGroup2DImGui(selection_.name);
		break;
	}

	case InspectorCategory::Particle:
	{
		auto* emitter = EmitterManager::GetInstance()->GetEmitter(selection_.name);

		if (emitter) {
			emitter->DrawImGui();
		}

		ParticleManager::GetInstance()->DrawParticleGroupImGui(selection_.name);
		break;
	}

	case InspectorCategory::DirectionalLight:
	{
		LightManager::GetInstance()->DrawDirectionalLightImGui(selection_.name);
		break;
	}

	case InspectorCategory::PointLight:
	{
		LightManager::GetInstance()->DrawPointLightImGui(selection_.name);
		break;
	}

	case InspectorCategory::SpotLight:
	{
		LightManager::GetInstance()->DrawSpotLightImGui(selection_.name);
		break;
	}

	case InspectorCategory::WorldField2D:
	{
		WorldField2DManager::GetInstance()->DrawImGui(selection_.name);
		break;
	}

	case InspectorCategory::WorldField:
	{
		WorldFieldManager::GetInstance()->DrawImGui(selection_.name);
		break;
	}

	default:
		ImGui::Text("Nothing selected.");
		break;
	}

	ImGui::End();
#endif
}
