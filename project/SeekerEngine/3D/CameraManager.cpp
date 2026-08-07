#include "CameraManager.h"
#include <algorithm>
#include <numbers>
#include "Logger.h"

CameraManager* CameraManager::GetInstance() {
	static CameraManager instance;
	return &instance;
}

void CameraManager::Init()
{
	CreateCamera("MainCamera");
	cameras_[0].camera->SetTranslate({0.0f, 23.0f, -10.0f});
	cameras_[0].camera->SetRotate(
		{ std::numbers::pi_v<float> / 3.0f,
		 0.0f,
		 0.0f }
	);
	cameras_[0].camera->SetScale({ 1.0f, 1.0f, 1.0f });
	CreateDebugCamera();
	SetActiveCamera(false, 0);
}

void CameraManager::Update()
{
	if (activeIsDebug_) {
		if (debugCamera_) {
			debugCamera_->Update();
		}
	} else {
		if (cameras_.empty() || activeCamIndex_ < 0 || activeCamIndex_ >= static_cast<int>(cameras_.size())) {
			return;
		}
		cameras_[activeCamIndex_].camera->Update();
	}
}

void CameraManager::Shutdown()
{
	cameras_.clear();
	debugCamera_.reset();
}	

void CameraManager::CreateCamera(const std::string& cameraName)
{
	for (int i = 0; i < cameras_.size(); ++i) {
		if (cameras_[i].name == cameraName) {
			return;
		}
	}

	CameraInfo cameraInfo;
	cameraInfo.name = cameraName;
	cameraInfo.camera = std::make_unique<Camera>();
	cameraInfo.camera->SetTranslate({0.0f, 10.0f, -10.0f});
	cameraInfo.camera->SetRotate({0.0f, 0.0f, 0.0f});
	cameras_.push_back(std::move(cameraInfo));
}

void CameraManager::CreateDebugCamera()
{
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
}

void CameraManager::SetActiveCamera(bool isDebug, int index)
{
	activeIsDebug_ = isDebug;
	if (!cameras_.empty()) {
		activeCamIndex_ = std::clamp(index, 0, static_cast<int>(cameras_.size()) - 1);
	} else {
		activeCamIndex_ = 0;
	}
}

void CameraManager::SetActiveCameraByName(const std::string& name)
{
	for (int i = 0; i < cameras_.size(); ++i) {
		if (cameras_[i].name == name) {
			activeCamIndex_ = i;
			activeIsDebug_ = false;
			return;
		}
	}
	assert(false && "指定された名前のカメラが存在しません");
}

Camera* CameraManager::GetActiveCamera() const
{
	if (activeIsDebug_) {
		return nullptr;
	}

	if (cameras_.empty() || activeCamIndex_ < 0 || activeCamIndex_ >= static_cast<int>(cameras_.size())) {
		return nullptr;
	}

	return  cameras_[activeCamIndex_].camera.get();
}

Camera* CameraManager::GetCamera(const std::string& name)
{
	for(auto& cam : cameras_) {
		if (cam.name == name) {
			return cam.camera.get();
		}
	}

	assert(false && "指定された名前のカメラが見つかりません");
	return nullptr;
}

nlohmann::json CameraManager::SaveToJson() const
{
	nlohmann::json j = nlohmann::json::array();

	for (const auto& info : cameras_) {
		nlohmann::json camJson;
		camJson["name"] = info.name;
		if (info.camera) {
			camJson["camera"] = info.camera->SaveToJson();
		}
		j.push_back(camJson);
	}

	return j;
}

void CameraManager::LoadFromJson(const nlohmann::json& j)
{
	if (!j.is_array()) {
		Logger::Write(Logger::LogLevel::Warning, "Not Camera Array");
		return;
	}

	if (j.empty()) {
		Logger::Write(Logger::LogLevel::Warning, "Camera Array is empty. Keep current cameras.");
		return;
	}

	std::vector<CameraInfo> loadedCameras;
	for (const auto& item : j) {
		if (!item.is_object() || !item.contains("camera") || !item["camera"].is_object()) {
			continue;
		}

		CameraInfo info;
		info.name = item.value("name", "untitled");

		info.camera = std::make_unique<Camera>();
		if (item["camera"].contains("transform")) {
			info.camera->LoadFromJson(item["camera"]);
		}
		loadedCameras.push_back(std::move(info));
	}

	if (loadedCameras.empty()) {
		Logger::Write(Logger::LogLevel::Warning, "No valid cameras in JSON. Keep current cameras.");
		return;
	}

	cameras_ = std::move(loadedCameras);
	SetActiveCamera(false, activeCamIndex_);
}
