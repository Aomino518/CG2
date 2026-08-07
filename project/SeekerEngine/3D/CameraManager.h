#pragma once
#include "Camera.h"
#include "DebugCamera.h"
#include <memory>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class CameraManager
{
public:
	static CameraManager* GetInstance();

	struct CameraInfo {
		std::string name;
		std::unique_ptr<Camera> camera;
	};

	void Init();
	void Update();
	void Shutdown();

	void CreateCamera(const std::string& cameraName);
	void CreateDebugCamera();

	void SetActiveCamera(bool isDebug, int index = 0);
	void SetActiveCameraByName(const std::string& name);

	const std::vector<CameraInfo>& GetCameras() const { return cameras_; }
	Camera* GetActiveCamera() const;
	bool GetIsDebug() const { return activeIsDebug_; }
	int GetActtiveCamIndex() const { return activeCamIndex_; }
	DebugCamera* GetDebugCamera() const { return debugCamera_.get(); }
	Camera* GetCamera(const std::string& name);

	// json保存と読み込み
	nlohmann::json SaveToJson() const;
	void LoadFromJson(const nlohmann::json& j);

private:
	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	std::vector<CameraInfo> cameras_;
	std::unique_ptr<DebugCamera> debugCamera_;

	bool activeIsDebug_ = false;
	int activeCamIndex_ = 0;
};

