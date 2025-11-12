#include "AssetBrowser.h"
#include <filesystem>

void AssetBrowser::LoadAssets(const std::string& directoryPath)
{
	textureFiles_.clear();

	for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
		auto exturn = entry.path().extension().string();
		if (exturn == ".png" || exturn == ".jpg" || exturn == ".jpeg") {
			textureFiles_.push_back(entry.path().string());
		}
	}
}

void AssetBrowser::Draw()
{
#ifdef USE_IMGUI
	int selectedIndex = -1;
	ImGui::Begin("Assets");
	for (int i = 0; i < textureFiles_.size(); i++) {
		std::string filename = std::filesystem::path(textureFiles_[i]).filename().string();

		if (ImGui::Selectable(filename.c_str(), selectedIndex == i)) {
			selectedIndex = i;
		}

		if (ImGui::BeginDragDropSource()) {
			const std::string& path = textureFiles_[i];
			ImGui::SetDragDropPayload("ASSET_TYPE", path.c_str(), path.size() + 1);
			ImGui::Text("Dragging %s", filename.c_str());
			ImGui::EndDragDropSource();
		}
	}
	ImGui::End();
#endif
}
