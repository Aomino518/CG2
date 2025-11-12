#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Sprite.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

class SceneView
{
public:
	void Draw(std::vector<std::unique_ptr<Sprite>>& sprites);
};

