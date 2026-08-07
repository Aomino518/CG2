#include "GameOverScene.h"
#include "SeekerEngine.h"
#include "SceneIncludes.h"

void GameOverScene::Init()
{
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("se_game_over", "resources/se_game_over.mp3");
	soundMgr->Load("se_selected", "resources/se_selected.mp3");
	soundMgr->PlaySE("se_game_over");

	//===========================
	// カメラマネージャー
	//===========================
	camMgr_ = std::make_unique<CameraManager>();
	camMgr_->Init();
	auto entityCommon = Entity3DCommon::GetInstance();
	entityCommon->SetCameraManager(camMgr_.get());
	entityCommon->SetDebugCamera(camMgr_->GetDebugCamera());
	entityCommon->SetDefaultCamera(camMgr_->GetActiveCamera());

	// カメラの初期位置設定
	auto camera = camMgr_->GetActiveCamera();
	Vector3 camPos = { 0.0f, 14.0f, -100.0f };
	Vector3 camRot = { 0.14f, 0.0f, 0.0f };
	camera->SetTranslate(camPos);
	camera->SetRotate(camRot);

	//===========================
	// テクスチャ
	//===========================
	auto texMgr = TextureManager::GetInstance();
	texUiGameOver_ = texMgr->Load("resources/ui_game_over.png");
	texUiBackTitle_ = texMgr->Load("resources/ui_space_backtitle.png");

	//===========================
	// スプライト
	//===========================
	sprGameOver_ = std::make_unique<Sprite>();
	sprGameOver_->Init();
	sprGameOver_->Create(texUiGameOver_, { 620.0f, 215.0f }, Color::WHITE);
	sprGameOver_->SetAnchorPoint({ 0.5f, 0.5f });

	sprUiSpaceBackTitle_ = std::make_unique<Sprite>();
	sprUiSpaceBackTitle_->Init();
	sprUiSpaceBackTitle_->Create(texUiBackTitle_, { 370.0f, 510.0f }, Color::WHITE);

	//===========================
	// モデル
	//===========================
	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("starSkyDome");
	modelSkydome_->SetIsLighting(false);

	//===========================
	// クラス
	//===========================
	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
	Logger::Write("現在シーンGameOverScene");

	ImGuiManager::GetInstance()->LoadScenesJson();
}

void GameOverScene::Update()
{
	auto soundMgr = SoundManager::GetInstance();
	/*-- 更新処理 --*/
	switch (phase_) {
	case ScenePhase::FADEIN:
		if (fade_.IsFinished()) {
			phase_ = ScenePhase::MAIN;
		}
		break;
	case ScenePhase::MAIN:
		if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
			soundMgr->PlaySE("se_selected");
			fade_.Start(Fade::Status::FadeOut, 1.0f);
			phase_ = ScenePhase::FADEOUT;
		}

		break;
	case ScenePhase::FADEOUT:
		if (fade_.IsFinished()) {
			soundMgr->StopSE();
			SceneManager::GetInstance()->ChangeScene("TITLE");
		}
		break;
	}

	// カメラの更新処理
	camMgr_->Update();

	// モデルの更新処理
	modelSkydome_->Update();
	
	// スプライトの更新処理
	sprGameOver_->Update();
	sprUiSpaceBackTitle_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
	sprUiSpaceBackTitle_->Update();
	fade_.Update();

	UpdateImGui();
	auto camMgr = CameraManager::GetInstance();

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->DrawCameraWindow(camMgr);
    ImGuiManager::GetInstance()->DrawEditor();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->DrawSoundWindow();
    ImGuiManager::GetInstance()->DrawLoggerWindow();
    ImGuiManager::GetInstance()->EndFrame();
}

void GameOverScene::Draw()
{
	/*-- 描画処理 --*/
	Entity3DCommon::GetInstance()->DrawCommon();
	modelSkydome_->Draw();

	SpriteCommon::GetInstance()->DrawCommon();
	sprGameOver_->Draw();
	sprUiSpaceBackTitle_->Draw();
	fade_.Draw();

	ImGuiManager::GetInstance()->Draw();
    ImGuiManager::GetInstance()->Draw();
}

void GameOverScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Unload("se_selected");
	soundMgr->Unload("se_game_over");
}

void GameOverScene::UpdateImGui() {
	auto imguiMgr = ImGuiManager::GetInstance();
	imguiMgr->BegineFrame();
	imguiMgr->BegineInspector();
	imguiMgr->SpriteSetting("GameOver", sprGameOver_.get());
	imguiMgr->SpriteSetting("UiSpaceBackTitle", sprUiSpaceBackTitle_.get());
	imguiMgr->ModelSetting("StarSkyDome", modelSkydome_.get());
	imguiMgr->EndInspector();
	imguiMgr->CameraSetting(camMgr_.get());
	imguiMgr->Stats();
	imguiMgr->EndFrame();
}
    Editor::GetInstance()->Clear();
}
