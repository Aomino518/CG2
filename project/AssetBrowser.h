#pragma once
#include <string>
#include <vector>
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

class AssetBrowser
{
public:
	void LoadAssets(const std::string& directoryPath);
	void Draw();
	const std::string& GetDraggedAsset() const { return draggedAsset_; }

private:
	std::vector<std::string> textureFiles_;
	std::string draggedAsset_;
};

