#pragma once
#include <stdlib.h>
#include <string>
#include <nlohmann/json.hpp>
#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#endif

enum class UITheme {
	Default,
	Cyberpunk
};

class Application;
class Graphics;
class CameraManager;
class ImGuiManager
{
public:
	static ImGuiManager* GetInstance();

	void Init();
	void BeginFrame();
	void EndFrame();
	void Draw();
	void Shutdown();

	void DrawSoundWindow();
	void DrawCameraWindow(CameraManager* cameraManager);
	void DrawMainMenuBar();
	void DrawLoggerWindow();

	void DrawEditor();

	void Stats();
	void ShowMemoryUsage();
	void BeginInspector();
	void EndInspector();
	void SaveScenesJson();
	void LoadScenesJson();
	void ClearScenesJson();
	void DrawConfirmPopup();

	nlohmann::json SaveEditorJson() const;
	void LoadEditorJson(const nlohmann::json& j);
	
private:

	// メンバ関数
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void ApplyStyle();
	void DrawDockSpace();

	// メンバ変数
	Application* app_ = nullptr;
	Graphics* graphics_ = nullptr;

	// 表示するWindowのフラグ
	struct WindowFlags {
		bool showStats = true;
		bool showCamera = true;
		bool showSound = true;
		bool showConsole = true;
	};

	WindowFlags windowState_;
	bool requestSavePopup_ = false;
	bool requestLoadPopup_ = false;
	bool requestClearPopup_ = false;
};

