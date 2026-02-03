#include "ClearScene.h"
#include "SeekerEngine.h"

void ClearScene::Init()
{
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("se_game_clear", "resources/se_game_clear.mp3");
	soundMgr->Load("se_selected", "resources/se_selected.mp3");
	soundMgr->PlaySE("se_game_clear");

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
	texUiGameClear_ = texMgr->Load("resources/ui_game_clear.png");
	texUiBackTitle_ = texMgr->Load("resources/ui_space_backtitle.png");

	//===========================
	// スプライト
	//===========================
	sprGameClear_ = std::make_unique<Sprite>();
	sprGameClear_->Init();
	sprGameClear_->Create(texUiGameClear_, { 620.0f, 215.0f }, Color::WHITE);
	sprGameClear_->SetAnchorPoint({ 0.5f, 0.5f });

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
}

void ClearScene::Update()
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
	sprGameClear_->Update();
	sprUiSpaceBackTitle_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
	sprUiSpaceBackTitle_->Update();
	fade_.Update();

	UpdateImGui();
}

void ClearScene::Draw()
{
	/*-- 描画処理 --*/
	Entity3DCommon::GetInstance()->DrawCommon();
	modelSkydome_->Draw();

	SpriteCommon::GetInstance()->DrawCommon();
	sprGameClear_->Draw();
	sprUiSpaceBackTitle_->Draw();
	fade_.Draw();

	ImGuiManager::GetInstance()->Draw();
}

void ClearScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Unload("se_selected");
	soundMgr->Unload("se_game_clear");
}

void ClearScene::UpdateImGui() {
	auto imguiMgr = ImGuiManager::GetInstance();
	imguiMgr->BegineFrame();
	imguiMgr->BegineInspector();
	imguiMgr->SpriteSetting("GameClear", sprGameClear_.get());
	imguiMgr->ModelSetting("StarSkyDome", modelSkydome_.get());
	imguiMgr->EndInspector();
	imguiMgr->CameraSetting(camMgr_.get());
	imguiMgr->Stats();
	imguiMgr->EndFrame();
}
