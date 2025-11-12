#include "ImGuiManager.h"
#include "Logger.h"
#include <Psapi.h>

void ImGuiManager::Init([[maybe_unused]] Application* app, [[maybe_unused]] Graphics* graphics)
{
#ifdef USE_IMGUI
	app_ = app;
	graphics_ = graphics;

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(app_->GetHWND());
	ImGui_ImplDX12_Init(graphics_->GetDevice(),
		graphics_->GetSwapChainDesc().BufferCount,
		graphics_->GetRTVDesc().Format,
		graphics_->GetSRVHeap().Get(),
		graphics_->GetSRVHeap()->GetCPUDescriptorHandleForHeapStart(),
		graphics_->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
	Logger::Write("ImGui初期化");

	// スタイルを設定
	StyleSetting();
#endif
}

void ImGuiManager::BegineFrame()
{
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::EndFrame()
{
#ifdef USE_IMGUI
	// ImGuiの内部コマンドを生成する
	ImGui::Render();
#endif
}

void ImGuiManager::Draw()
{
#ifdef USE_IMGUI
	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), graphics_->GetCmdList());
#endif
}

void ImGuiManager::Shutdown()
{
#ifdef USE_IMGUI
	// ImGuiの終了処理。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void ImGuiManager::SpriteSetting(const std::string& spriteName, Sprite* sprite)
{
#ifdef USE_IMGUI
	Vector4 spriteMaterial = sprite->GetColor();
	Vector2 spritePosition = sprite->GetPosition();
	float spriteRotate = sprite->GetRotation();
	Vector2 spriteScale = sprite->GetSize();

	if (ImGui::CollapsingHeader(spriteName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID(spriteName.c_str());
		if (ImGui::TreeNode("Material"))
		{
			ImGui::ColorEdit4("Color", (float*)&spriteMaterial);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Transform"))
		{
			ImGui::DragFloat2("Position", (float*)&spritePosition, 1.00f, -1280.0f, 1280.0f, "%.2f");
			ImGui::SliderAngle("Rotation", (float*)&spriteRotate, 0.0f, 360.0f, "%.3f");
			ImGui::DragFloat2("Scale", (float*)&spriteScale, 1.0f, 0.0f, 1280.0f, "%.2f");
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	sprite->SetColor(spriteMaterial);
	sprite->SetPosition(spritePosition);
	sprite->SetRotation(spriteRotate);
	sprite->SetSize(spriteScale);
#endif
}

void ImGuiManager::ModelSetting(const std::string& modelName, Entity3D* model)
{
#ifdef USE_IMGUI
	Vector4 modelMaterial = model->GetMaterial();
	Vector3 modelPosition = model->GetTranslate();
	Vector3 modelRotate = model->GetRotate();
	Vector3 modelScale = model->GetScale();

	if (ImGui::CollapsingHeader(modelName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID(modelName.c_str());
		if (ImGui::TreeNode("Material"))
		{
			ImGui::ColorEdit4("Color", (float*)&modelMaterial);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Transform"))
		{
			ImGui::DragFloat3("Position", (float*)&modelPosition, 0.01f, -1280.0f, 1280.0f, "%.2f");
			ImGui::DragFloat3("Rotation", (float*)&modelRotate, 0.01f, -360.0f, 360.0f, "%.2f");
			ImGui::DragFloat3("Scale", (float*)&modelScale, 0.01f, 0.0f, 1280.0f, "%.2f");
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	model->SetMaterial(modelMaterial);
	model->SetTranslate(modelPosition);
	model->SetRotate(modelRotate);
	model->SetScale(modelScale);
	
#endif
}

void ImGuiManager::Stats()
{
#ifdef USE_IMGUI
	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
	ShowMemoryUsage();
	ImGui::End();
#endif
}

void ImGuiManager::ShowMemoryUsage() {
#ifdef USE_IMGUI
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		// メモリ使用量をMB単位で計算
		double memoryUsageMB = pmc.WorkingSetSize / (1024.0 * 1024.0);

		ImGui::Text("Memory Usage: %.2f MB", memoryUsageMB);
	}
#endif
}

void ImGuiManager::BegineInspector()
{
#ifdef USE_IMGUI
	ImGui::Begin("Inspector");
#endif
}

void ImGuiManager::EndInspector()
{
#ifdef USE_IMGUI
	ImGui::End();
#endif
}

void ImGuiManager::CameraSetting(Vector3& positoin, Vector3& rotation)
{
#ifdef USE_IMGUI
	if (ImGui::CollapsingHeader("DefaultCamera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Position", (float*)&positoin, 0.01f, -10.0f, 10.0f, "%.2f");
		ImGui::DragFloat3("Rotation", (float*)&rotation, 0.01f, -360.0f, 360.0f, "%.2f");
	}
#endif
}

void ImGuiManager::AssetBrowserDraw()
{
#ifdef USE_IMGUI
	assetBrowser->Draw();
#endif
}

void ImGuiManager::SceneViewDraw(std::vector<std::unique_ptr<Sprite>>& sprites)
{
#ifdef USE_IMGUI
	sceneView->Draw(sprites);
#endif
}

void ImGuiManager::Inspector(std::vector<std::unique_ptr<Sprite>>& sprites)
{
#ifdef USE_IMGUI
	ImGui::Begin("Imspector", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Menu")) {
			if (ImGui::MenuItem("Create")) {

			}

			if (ImGui::MenuItem("Delete")) {

			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	for (int i = 0; i < sprites.size(); i++) {
		std::string label = "Sprite" + std::to_string(i);
		if (ImGui::CollapsingHeader(label.c_str())) {
			ImGui::PushID(label.c_str());
			Vector2 pos = sprites[i]->GetPosition();
			float rotation = sprites[i]->GetRotation();
			Vector2 scale = sprites[i]->GetSize();
			ImGui::DragFloat2("Position", (float*)&pos, 1.0f, 0.0f, 1280.0f, "%0.1f");
			ImGui::SliderAngle("Rotation", (float*)&rotation, 0.0f, 360.0f, "%.3f");
			ImGui::DragFloat2("Scale", (float*)&scale, 1.0f, 0.0f, 1280.0f, "%.1f");
			sprites[i]->SetPosition(pos);
			sprites[i]->SetRotation(rotation);
			sprites[i]->SetSize(scale);
			ImGui::PopID();
		}
	}
	ImGui::End();
#endif
}

void ImGuiManager::LoadAssets(const std::string& directoryPath)
{
#ifdef USE_IMGUI
	assetBrowser->LoadAssets(directoryPath);
#endif
}

void ImGuiManager::StyleSetting()
{
#ifdef USE_IMGUI
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* c = style.Colors;

	// --- 基本背景・テキスト ---
	c[ImGuiCol_Text] = ImVec4(0.80f, 0.90f, 1.00f, 1.00f);
	c[ImGuiCol_TextDisabled] = ImVec4(0.30f, 0.45f, 0.55f, 1.00f);
	c[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.03f, 0.05f, 1.00f);
	c[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.04f, 0.07f, 1.00f);
	c[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.07f, 0.10f, 0.98f);
	c[ImGuiCol_Border] = ImVec4(0.10f, 0.30f, 0.45f, 0.60f);
	c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// --- フレーム（入力欄、スライダーなど） ---
	c[ImGuiCol_FrameBg] = ImVec4(0.03f, 0.06f, 0.10f, 1.00f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.05f, 0.18f, 0.28f, 1.00f);
	c[ImGuiCol_FrameBgActive] = ImVec4(0.07f, 0.25f, 0.38f, 1.00f);

	// --- タイトルバー・ウィンドウ装飾 ---
	c[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.10f, 0.15f, 1.00f);
	c[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.25f, 0.35f, 1.00f);
	c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.10f, 0.15f, 0.70f);
	c[ImGuiCol_MenuBarBg] = ImVec4(0.02f, 0.08f, 0.12f, 1.00f);

	// --- ボタン ---
	c[ImGuiCol_Button] = ImVec4(0.00f, 0.40f, 0.55f, 0.90f);
	c[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.55f, 0.75f, 1.00f);
	c[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.60f, 0.90f, 1.00f);

	// --- ヘッダー（ツリーノード、リスト選択など） ---
	c[ImGuiCol_Header] = ImVec4(0.00f, 0.30f, 0.45f, 0.80f);
	c[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.40f, 0.55f, 1.00f);
	c[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.55f, 0.75f, 1.00f);

	// --- スクロールバー ---
	c[ImGuiCol_ScrollbarBg] = ImVec4(0.01f, 0.02f, 0.03f, 1.00f);
	c[ImGuiCol_ScrollbarGrab] = ImVec4(0.05f, 0.25f, 0.35f, 1.00f);
	c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.08f, 0.40f, 0.55f, 1.00f);
	c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.10f, 0.55f, 0.75f, 1.00f);

	// --- スライダー ---
	c[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.55f, 0.80f, 1.00f);
	c[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.70f, 1.00f, 1.00f);

	// --- チェックマーク ---
	c[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.75f, 1.00f, 1.00f);

	// --- セパレーター ---
	c[ImGuiCol_Separator] = ImVec4(0.10f, 0.30f, 0.45f, 0.80f);
	c[ImGuiCol_SeparatorHovered] = ImVec4(0.15f, 0.45f, 0.65f, 1.00f);
	c[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.60f, 0.85f, 1.00f);

	// --- リサイズグリップ ---
	c[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.45f, 0.65f, 0.60f);
	c[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.60f, 0.85f, 0.80f);
	c[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.75f, 1.00f, 1.00f);

	// --- タブ ---
	c[ImGuiCol_Tab] = ImVec4(0.00f, 0.25f, 0.35f, 0.90f);
	c[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.40f, 0.60f, 1.00f);
	c[ImGuiCol_TabActive] = ImVec4(0.00f, 0.55f, 0.80f, 1.00f);
	c[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.15f, 0.20f, 0.80f);
	c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.00f, 0.30f, 0.45f, 0.90f);

	// --- ドッキング・テーブル ---
	c[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.20f, 0.30f, 1.00f);
	c[ImGuiCol_TableBorderStrong] = ImVec4(0.10f, 0.35f, 0.55f, 1.00f);
	c[ImGuiCol_TableBorderLight] = ImVec4(0.05f, 0.25f, 0.40f, 1.00f);
	c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	c[ImGuiCol_TableRowBgAlt] = ImVec4(0.03f, 0.05f, 0.08f, 0.50f);

	// --- プロット・ドラッグ・ドロップ ---
	c[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.70f, 1.00f, 1.00f);
	c[ImGuiCol_PlotLinesHovered] = ImVec4(0.20f, 0.90f, 1.00f, 1.00f);
	c[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.60f, 0.90f, 1.00f);
	c[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
	c[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.75f, 1.00f, 0.90f);

	// --- ナビゲーション・モーダル背景 ---
	c[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.60f, 0.90f, 1.00f);
	c[ImGuiCol_NavWindowingHighlight] = ImVec4(0.00f, 0.55f, 0.80f, 0.70f);
	c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.05f, 0.10f, 0.50f);
	c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.02f, 0.05f, 0.80f);

	// --- テーマ全体の丸み・間隔調整 ---
	style.WindowRounding = 6.0f;
	style.FrameRounding = 5.0f;
	style.GrabRounding = 4.0f;
	style.ScrollbarRounding = 8.0f;
	style.TabRounding = 5.0f;
	style.FramePadding = ImVec2(6.0f, 4.0f);
	style.ItemSpacing = ImVec2(6.0f, 6.0f);
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.5f;
#endif
}
