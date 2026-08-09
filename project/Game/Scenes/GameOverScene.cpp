#include "GameOverScene.h"
#include "SeekerEngine.h"
#include "SceneIncludes.h"

void GameOverScene::Init()
{
	Logger::Write("現在シーンGameOverScene");
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("se_game_over", "se_game_over.mp3");
	soundMgr->Load("se_selected", "se_selected.mp3");
	soundMgr->PlaySE("se_game_over");

	//===========================
	// カメラマネージャー
	//===========================
	auto camMgr = CameraManager::GetInstance();
	auto entityCommon = Entity3DCommon::GetInstance();
	entityCommon->SetCameraManager(camMgr);
	entityCommon->SetDebugCamera(camMgr->GetDebugCamera());
	entityCommon->SetDefaultCamera(camMgr->GetActiveCamera());

	//===========================
	// テクスチャ
	//===========================
	auto texMgr = TextureManager::GetInstance();
	texUiGameOver_ = texMgr->Load("resources/sprites/ui_game_over.png");
	texUiBackTitle_ = texMgr->Load("resources/sprites/ui_space_backtitle.png");

	//===========================
	// スプライト
	//===========================
	sprGameOver_ = std::make_unique<Sprite>();
	sprGameOver_->Init();
	sprGameOver_->Create(texUiGameOver_, { 620.0f, 215.0f }, Color::WHITE);
	sprGameOver_->SetAnchorPoint({ 0.5f, 0.5f });
	Editor::GetInstance()->RegisterSprite("sprGameOver", sprGameOver_.get());

	sprUiSpaceBackTitle_ = std::make_unique<Sprite>();
	sprUiSpaceBackTitle_->Init();
	sprUiSpaceBackTitle_->Create(texUiBackTitle_, { 370.0f, 510.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiSpaceBackTitle", sprUiSpaceBackTitle_.get());

	//===========================
	// モデル
	//===========================
	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("starSkyDome");
	Editor::GetInstance()->RegisterModel("starSkyDome", modelSkydome_.get());
	
	//===========================
	// クラス
	//===========================
	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
	ImGuiManager::GetInstance()->LoadScenesJson();
}

void GameOverScene::Update()
{
	auto camMgr = CameraManager::GetInstance();
	auto soundMgr = SoundManager::GetInstance();
	/*-- 更新処理 --*/
	switch (phase_) {
	case ScenePhase::FADEIN:
		if (fade_.IsFinished()) {
			phase_ = ScenePhase::MAIN;
		}
		break;
	case ScenePhase::MAIN:
		if (Input::GetInstance()->IsPress(DIK_SPACE)) {
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
	camMgr->Update();

	// モデルの更新処理
	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelSkydome_->SetCamera(camera);
	}

	modelSkydome_->Update();

	// スプライトの更新処理
	sprGameOver_->Update();
	sprUiSpaceBackTitle_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
	sprUiSpaceBackTitle_->Update();
	fade_.Update();

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
	// Model
	modelSkydome_->Draw();

	// Sprite
	sprGameOver_->Draw();
	sprUiSpaceBackTitle_->Draw();
	fade_.Draw();

    ImGuiManager::GetInstance()->Draw();
}

void GameOverScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Unload("se_selected");
	soundMgr->Unload("se_game_over");
    Editor::GetInstance()->Clear();
}