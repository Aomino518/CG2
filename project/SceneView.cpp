#include "SceneView.h"

void SceneView::Draw(std::vector<std::unique_ptr<Sprite>>& sprites)
{
#ifdef USE_IMGUI
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::SetNextWindowPos(ImVec2(250, 30), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);

	ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_NoCollapse);

	// ウィンドウ内利用可能サイズを取得
	ImVec2 region = ImGui::GetContentRegionAvail();

	ImGui::InvisibleButton("SceneDropZone", region);

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TYPE")) {
			const char* filePath = (const char*)payload->Data;
			// ドロップされた位置取得
			ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 windouwPos = ImGui::GetWindowPos();
			Vector2 localPos = { mousePos.x - windouwPos.x, mousePos.y - windouwPos.y };

			// Texture読み込み
			uint32_t textureHandle = TextureManager::GetInstance()->Load(filePath);

			std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
			sprite->Create(textureHandle, localPos, Color::WHITE);
			sprites.push_back(std::move(sprite));
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::End();
#endif
}
