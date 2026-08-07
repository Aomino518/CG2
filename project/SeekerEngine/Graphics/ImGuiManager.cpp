#include "ImGuiManager.h"
#include "ImGuiUtils.h"
#include "Logger.h"
#include <Psapi.h>
#include "Application.h"
#include "Graphics.h"
#include "SoundManager.h"
#include "LightManager.h"
#include "EmitterManager.h"
#include "SceneManager.h"
#include "ParticleManager.h"
#include "Particle2DManager.h"
#include "Emitter2DManager.h"
#include "SrvManager.h"
#include "Editor.h"
#include "Vector3.h"
#include <nlohmann/json.hpp>

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Init()
{
#ifdef USE_IMGUI
	app_ = Application::GetInstance();
	graphics_ = Graphics::GetInstance();

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	io.Fonts->AddFontFromFileTTF(
		"resources/fonts/NotoSansJP-Regular.ttf",
		18.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese()
	);

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(app_->GetHWND());
	ImGui_ImplDX12_Init(graphics_->GetDevice(),
		graphics_->GetSwapChainDesc().BufferCount,
		graphics_->GetRTVDesc().Format,
		SrvManager::GetInstance()->GetDiscriptorHeap(),
		SrvManager::GetInstance()->GetDiscriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		SrvManager::GetInstance()->GetDiscriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);
	Logger::Write("ImGui初期化");

	// スタイルを設定
	ApplyStyle();
#endif
}

void ImGuiManager::BeginFrame()
{
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	DrawDockSpace();
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
	Logger::Write("ImGuiManager Shutdown");
#endif
}

void ImGuiManager::Stats()
{
#ifdef USE_IMGUI
	if (windowState_.showStats) {
		ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::SeparatorText("Frame");
		float fps = ImGui::GetIO().Framerate;
		ImGui::Text("FPS: %.2f", fps);
		ImGui::Text("Frame Time: %.2f ms", 1000.0f / fps);

		ImGui::SeparatorText("Rendering");
		ImGui::Text("DrawCalls: %u", Graphics::GetInstance()->GetDrawCallCount());

		ImGui::SeparatorText("Particle");
		ImGui::Text("Particles: %u", ParticleManager::GetInstance()->GetTotalParticleCount());
		ImGui::Text("Particle Groups: %u", ParticleManager::GetInstance()->GetParticleGroupCount());
		ImGui::Text("Emitters: %u", EmitterManager::GetInstance()->GetEmitterCount());

		ImGui::SeparatorText("Particle2D");
		ImGui::Text("Particles: %u", Particle2DManager::GetInstance()->GetTotalParticleCount());
		ImGui::Text("Particle Groups: %u", Particle2DManager::GetInstance()->GetParticleGroupCount());
		ImGui::Text("Emitters: %u", Emitter2DManager::GetInstance()->GetEmitterCount());

		ImGui::SeparatorText("Memory");
		ShowMemoryUsage();
		ImGui::End();
	}
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

void ImGuiManager::BeginInspector()
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

void ImGuiManager::DrawCameraWindow(CameraManager* cameraManager)
{
#ifdef USE_IMGUI
	if (windowState_.showCamera) {
		ImGui::Begin("Camera Manager");

		if (ImGui::CollapsingHeader("Debug Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool selected = cameraManager->GetIsDebug();
			if (ImGui::Selectable("DebugCamera", selected)) {
				cameraManager->SetActiveCamera(true);
			}
		}

		if (ImGui::CollapsingHeader("Normal Cameras", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& cameras = cameraManager->GetCameras();

			for (int i = 0; i < cameras.size(); i++)
			{
				bool selected = (!cameraManager->GetIsDebug() && cameraManager->GetActtiveCamIndex() == i);
				Vector3 pos = cameras[i].camera->GetTranslate();
				Vector3 rot = cameras[i].camera->GetRotate();

				if (ImGui::Selectable(cameras[i].name.c_str(), selected))
				{
					cameraManager->SetActiveCamera(false, i);
				}

				// 展開してパラメータ編集
				if (selected)
				{
					ImGui::PushID(cameras[i].name.c_str());
					ImGui::Indent();

					bool changed = false;
					changed |= ImGui::DragFloat3("Position", (float*)&pos, 0.01f);
					changed |= ImGui::DragFloat3("Rotation", (float*)&rot, 0.01f);

					if (changed) {
						cameras[i].camera->SetTranslate(pos);
						cameras[i].camera->SetRotate(rot);
					}

					ImGui::Unindent();
					ImGui::PopID();
				}
			}
		}

		ImGui::End();
	}

#endif
}

void ImGuiManager::DrawMainMenuBar()
{
#ifdef USE_IMGUI
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				requestSavePopup_ = true;
			}

			if (ImGui::MenuItem("Load")) {
				requestLoadPopup_ = true;
			}

			if (ImGui::MenuItem("Clear")) {
				requestClearPopup_ = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			ImGui::MenuItem("Stats", nullptr, &windowState_.showStats);
			ImGui::MenuItem("Camera", nullptr, &windowState_.showCamera);
			ImGui::MenuItem("Sound", nullptr, &windowState_.showSound);
			ImGui::MenuItem("Console", nullptr, &windowState_.showConsole);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (requestSavePopup_) {
		ImGui::OpenPopup("Confirm Save");
		requestSavePopup_ = false;
	}

	if (requestLoadPopup_) {
		ImGui::OpenPopup("Confirm Load");
		requestLoadPopup_ = false;
	}

	if (requestClearPopup_) {
		ImGui::OpenPopup("Confirm Clear");
		requestClearPopup_ = false;
	}

	DrawConfirmPopup();
#endif
}

void ImGuiManager::DrawLoggerWindow()
{
#ifdef USE_IMGUI
	if (windowState_.showConsole) {
		ImGui::Begin("Console");

		if (ImGui::Button("Clear")) {
			Logger::ClearHistory();
		}

		ImGui::SameLine();

		static bool autoScroll = true;
		ImGui::Checkbox("Auto Scroll", &autoScroll);

		ImGui::Separator();

		ImGui::BeginChild("Log Region", ImVec2(0, 0), true);

		const auto logs = Logger::GetHistory();
		for (const auto& log : logs) {
			ImGui::TextUnformatted(log.c_str());
		}

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}
#endif
}

void ImGuiManager::DrawEditor()
{
	Editor::GetInstance()->Draw();
}

void ImGuiManager::DrawSoundWindow()
{
#ifdef USE_IMGUI
	if (windowState_.showSound) {
		ImGui::Begin("Sound Manager");

		SoundManager* soundManager = SoundManager::GetInstance();
		auto volumeBGM = soundManager->GetCurrentBGMVolume();
		auto volumeSE = soundManager->GetCurrentSEVolume();
		auto volumeMaster = soundManager->GetCurrentMasterVolume();

		if (ImGui::DragFloat("BGM Volume", &volumeBGM, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeBGM(volumeBGM);
		}

		if (ImGui::DragFloat("SE Volume", &volumeSE, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeSE(volumeSE);
		}

		if (ImGui::DragFloat("Master Volume", &volumeMaster, 0.01f, 0.0f, 1.0f, "%.2f")) {
			soundManager->SetVolumeMaster(volumeMaster);
		}

		ImGui::End();
	}
#endif
}

void ImGuiManager::ApplyStyle()
{
#ifdef USE_IMGUI
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* c = style.Colors;

	// --- 基本背景・テキスト ---
	c[ImGuiCol_Text] = ImVec4(0.90f, 0.96f, 1.00f, 1.00f);
	c[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.55f, 0.65f, 1.00f);

	c[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.96f);
	c[ImGuiCol_ChildBg] = ImVec4(0.02f, 0.02f, 0.03f, 1.00f);
	c[ImGuiCol_PopupBg] = ImVec4(0.02f, 0.02f, 0.03f, 1.00f);

	c[ImGuiCol_Border] = ImVec4(0.20f, 0.70f, 1.00f, 0.80f);
	c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// --- フレーム（入力欄、スライダーなど） ---
	c[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.24f, 0.33f, 1.00f);
	c[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.31f, 0.42f, 1.00f);

	// --- タイトルバー・ウィンドウ装飾 ---
	c[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.15f, 0.22f, 1.00f);
	c[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.32f, 0.46f, 1.00f);
	c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.12f, 0.18f, 0.75f);
	c[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.12f, 0.18f, 1.00f);

	// --- ボタン ---
	c[ImGuiCol_Button] = ImVec4(0.00f, 0.42f, 0.60f, 1.00f);
	c[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.00f, 0.25f, 1.00f);
	c[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.80f, 0.00f, 1.00f);

	// --- ヘッダー（ツリーノード、リスト選択など） ---
	c[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.03f, 0.05f, 0.90f);
	c[ImGuiCol_Header] = ImVec4(0.00f, 0.40f, 0.60f, 0.40f); // 非選択時
	c[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.60f, 0.90f, 0.80f); // ホバーで発光
	c[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.80f, 1.00f, 1.00f); // 選択中で強発光

	// --- スクロールバー ---
	c[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.05f, 0.07f, 1.00f);
	c[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.25f, 0.35f, 1.00f);
	c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.38f, 0.52f, 1.00f);
	c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.50f, 0.68f, 1.00f);

	// --- スライダー ---
	c[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.00f, 0.25f, 1.00f);
	c[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.35f, 0.00f, 1.00f);

	// --- チェックマーク ---
	c[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.80f, 0.00f, 1.00f);

	// --- セパレーター ---
	c[ImGuiCol_Separator] = ImVec4(0.20f, 0.70f, 1.00f, 0.60f);
	c[ImGuiCol_SeparatorHovered] = ImVec4(0.28f, 0.52f, 0.72f, 1.00f);
	c[ImGuiCol_SeparatorActive] = ImVec4(0.38f, 0.68f, 0.90f, 1.00f);

	// --- リサイズグリップ ---
	c[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.52f, 0.74f, 0.55f);
	c[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.66f, 0.92f, 0.80f);
	c[ImGuiCol_ResizeGripActive] = ImVec4(0.20f, 0.80f, 1.00f, 1.00f);

	// --- タブ ---
	c[ImGuiCol_Tab] = ImVec4(0.06f, 0.20f, 0.28f, 1.00f);
	c[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.45f, 0.65f, 1.00f);
	c[ImGuiCol_TabActive] = ImVec4(0.00f, 0.58f, 0.82f, 1.00f);
	c[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.13f, 0.18f, 1.00f);
	c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.26f, 0.36f, 1.00f);

	// --- ドッキング・テーブル ---
	c[ImGuiCol_TableHeaderBg] = ImVec4(0.06f, 0.18f, 0.26f, 1.00f);
	c[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.40f, 0.56f, 1.00f);
	c[ImGuiCol_TableBorderLight] = ImVec4(0.10f, 0.24f, 0.34f, 1.00f);
	c[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	c[ImGuiCol_TableRowBgAlt] = ImVec4(0.07f, 0.10f, 0.14f, 0.60f);

	// --- プロット・ドラッグ・ドロップ ---
	c[ImGuiCol_PlotLines] = ImVec4(0.20f, 0.78f, 1.00f, 1.00f);
	c[ImGuiCol_PlotLinesHovered] = ImVec4(0.50f, 0.90f, 1.00f, 1.00f);
	c[ImGuiCol_PlotHistogram] = ImVec4(0.10f, 0.72f, 0.95f, 1.00f);
	c[ImGuiCol_PlotHistogramHovered] = ImVec4(0.35f, 0.85f, 1.00f, 1.00f);
	c[ImGuiCol_DragDropTarget] = ImVec4(0.20f, 0.85f, 1.00f, 1.00f);

	// --- ナビゲーション・モーダル背景 ---
	c[ImGuiCol_NavHighlight] = ImVec4(0.20f, 0.78f, 1.00f, 1.00f);
	c[ImGuiCol_NavWindowingHighlight] = ImVec4(0.20f, 0.78f, 1.00f, 0.70f);
	c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.03f, 0.06f, 0.60f);
	c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.02f, 0.05f, 0.82f);

	// --- テーマ全体の丸み・間隔調整 ---
	style.WindowRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.ChildRounding = 2.0f;
	style.PopupRounding = 2.0f;
	style.ScrollbarRounding = 3.0f;
	style.GrabRounding = 2.0f;
	style.TabRounding = 2.0f;

	// 余白を少しコンパクトに
	style.WindowPadding = ImVec2(6.0f, 5.0f);
	style.FramePadding = ImVec2(5.0f, 2.0f);
	style.ItemSpacing = ImVec2(5.0f, 3.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 2.0f);

	// インデント・スクロールバー
	style.IndentSpacing = 14.0f;
	style.ScrollbarSize = 10.0f;
	style.GrabMinSize = 8.0f;

	// 境界線
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.TabBorderSize = 0.0f;

#endif
}

void ImGuiManager::DrawDockSpace()
{
#ifdef USE_IMGUI
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGuiWindowFlags windowFlags = 
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace", nullptr, windowFlags);

	ImGui::PopStyleVar(3);

	ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

	ImGui::End();
#endif
}

void ImGuiManager::DrawConfirmPopup()
{
#ifdef USE_IMGUI
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Confirm Save", nullptr, flags)) {
		ImGui::Text("現在のシーンを保存しますか？");

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			SaveScenesJson();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Confirm Load", nullptr, flags)) {
		ImGui::Text("現在の編集内容を破棄してロードしますか？");

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			LoadScenesJson();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Confirm Clear", nullptr, flags)) {
		ImGui::Text("現在のシーンの保存内容を破棄しますか？");

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ClearScenesJson();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
#endif
}

void ImGuiManager::SaveScenesJson()
{
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->SaveSceneJson("resources/json/clearScene.json");
	}
}

void ImGuiManager::LoadScenesJson() {
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->LoadSceneJson("resources/json/clearScene.json");
	}
}

void ImGuiManager::ClearScenesJson()
{
	if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("TITLE")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/titleScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEPLAY")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/playScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("GAMEOVER")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/gameOverScene.json");
	} else if (SceneManager::GetInstance()->GetCurrentSceneName() == std::string("CLAER")) {
		Editor::GetInstance()->ClearSceneJson("resources/json/clearScene.json");
	}
}

json ImGuiManager::SaveEditorJson() const
{
	return nlohmann::json{
			{"showStats", windowState_.showStats},
			{"showCamera", windowState_.showCamera},
			{"showSound", windowState_.showSound},
	};
}

void ImGuiManager::LoadEditorJson(const nlohmann::json& j)
{
	if (j.contains("showStats")) {
		windowState_.showStats = j["showStats"].get<bool>();
	}

	if (j.contains("showCamera")) {
		windowState_.showCamera = j["showCamera"].get<bool>();
	}

	if (j.contains("showSound")) {
		windowState_.showSound = j["showSound"].get<bool>();
	}

}