#pragma once
#include "Graphics.h"
#include "Application.h"
#include <stdlib.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Sprite.h"
#include "Entity3D.h"
#include "AssetBrowser.h"
#include "SceneView.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

class ImGuiManager
{
public:
	void Init(Application* app, Graphics* graphics);
	void BegineFrame();
	void EndFrame();
	void Draw();
	void Shutdown();

	void SpriteSetting(const std::string& spriteName, Sprite* sprite);
	void ModelSetting(const std::string& modelName, Entity3D* model);
	void Stats();
	void ShowMemoryUsage();
	void BegineInspector();
	void EndInspector();
	void CameraSetting(Vector3& positoin, Vector3& rotation);
	void AssetBrowserDraw();
	void SceneViewDraw(std::vector<std::unique_ptr<Sprite>>& sprites);
	void Inspector(std::vector<std::unique_ptr<Sprite>>& sprites);
	void LoadAssets(const std::string& directoryPath);

private:
	void StyleSetting();

	Application* app_ = nullptr;
	Graphics* graphics_ = nullptr;
	std::unique_ptr<AssetBrowser> assetBrowser = std::make_unique<AssetBrowser>();
	std::unique_ptr<SceneView> sceneView = std::make_unique<SceneView>();
};

