#include "TitleScene.h"
#include "SceneIncludes.h"

void TitleScene::Init()
{
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("bgm_title", "resources/bgm_title.wav");
	soundMgr->Load("se_selected", "resources/se_selected.mp3");
	soundMgr->PlayBGM("bgm_title");

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
	camPos_ = { 0.0f, 7.3f, -50.0f };
	camRot_ = { 0.14f, 0.0f, 0.0f };
	camera->SetTranslate(camPos_);
	camera->SetRotate(camRot_);

	//===========================
	// テクスチャ
	//===========================
	auto texMgr = TextureManager::GetInstance();
	texTitleLogo_ = texMgr->Load("resources/spr_title_logo.png");
	texPressSpace_ = texMgr->Load("resources/ui_press_space.png");
	texMaou_ = texMgr->Load("resources/ui_maou.png");

	//===========================
	// モデルロード
	//===========================
	auto modelMgr = ModelManager::GetInstance();
	modelMgr->LoadModel("player");
	modelMgr->LoadModel("starSkyDome");

	//===========================
	// スプライト
	//===========================
	sprTitleLogo_ = std::make_unique<Sprite>();
	sprTitleLogo_->Init();
	sprTitleLogo_->Create(texTitleLogo_, { 385.0f, 31.0f }, Color::WHITE, { 500.0f, 290.0f });

	sprUiPressSpace_ = std::make_unique<Sprite>();
	sprUiPressSpace_->Init();
	sprUiPressSpace_->Create(texPressSpace_, { 415.0f, 575.0f }, Color::WHITE);

	sprUiMaou_ = std::make_unique<Sprite>();
	sprUiMaou_->Init();
	sprUiMaou_->Create(texMaou_, { 1070.0f, 665.0f }, Color::WHITE);

	//===========================
	// モデル
	//===========================
	modelPlayer_ = std::make_unique<Entity3D>();
	modelPlayer_->Init();
	modelPlayer_->SetModel("player");
	modelPlayer_->SetIsLighting(false);

	modelSkydome_ = std::make_unique<Entity3D>();
	modelSkydome_->Init();
	modelSkydome_->SetModel("starSkyDome");
	modelSkydome_->SetIsLighting(false);

	//===========================
	// クラス
	//===========================
	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
    Logger::Write("現在シーンTitleScene");
   
    ImGuiManager::GetInstance()->LoadScenesJson();
}

void TitleScene::Update()
{
	auto soundMgr = SoundManager::GetInstance();
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
			soundMgr->StopBGM();
			SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
		}
		break;
	}

	// カメラの更新処理
	UpdateCamera();
	camMgr_->Update();

	// モデルの更新処理
	modelPlayer_->Update();
	modelSkydome_->Update();

	// スプライトの更新処理
	sprTitleLogo_->Update();
	sprUiPressSpace_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
	sprUiPressSpace_->Update();
	sprUiMaou_->Update();
	fade_.Update();

	// ImGuiの更新処理
	UpdateImGui();
    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->DrawCameraWindow(camMgr);
    ImGuiManager::GetInstance()->DrawEditor();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->DrawSoundWindow();
    ImGuiManager::GetInstance()->DrawLoggerWindow();
    ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::Draw()
{
	// モデルの描画処理
	Entity3DCommon::GetInstance()->DrawCommon();
	modelPlayer_->Draw();
	modelSkydome_->Draw();

	// スプライトの描画処理
	SpriteCommon::GetInstance()->DrawCommon();
	sprTitleLogo_->Draw();
	if (phase_ != ScenePhase::FADEOUT) {
		sprUiPressSpace_->Draw();
	}
	sprUiMaou_->Draw();
	fade_.Draw();

	// ImGuiの描画処理
	ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Unload("bgm_title");
	soundMgr->Unload("se_selected");
}

void TitleScene::UpdateImGui()
{
	auto imguiMgr = ImGuiManager::GetInstance();
	imguiMgr->BegineFrame();
	imguiMgr->BegineInspector();
	imguiMgr->SpriteSetting("TitleLogo", sprTitleLogo_.get());
	imguiMgr->SpriteSetting("UiPressSpace", sprUiPressSpace_.get());
	imguiMgr->SpriteSetting("UiMaou", sprUiMaou_.get());
	imguiMgr->ModelSetting("Player", modelPlayer_.get());
	imguiMgr->ModelSetting("StarSkyDome", modelSkydome_.get());
	imguiMgr->EndInspector();
	imguiMgr->CameraSetting(camMgr_.get());
	imguiMgr->Stats();
	imguiMgr->EndFrame();
}

void TitleScene::UpdateCamera()
{
	auto camera = camMgr_->GetActiveCamera();
	switch (camPhase_) {
	case CameraPhase::BACK:
		camPos_.z += camSpeed_;
		frameCount_ += 1;

		if (frameCount_ >= 300) {
			camPos_ = { -30.0f, 7.3f, 30.0f };
			camRot_ = { 0.14f, 90.0f, 0.0f };
			frameCount_ = 0;
			camPhase_ = CameraPhase::LEFTSIDE;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	case CameraPhase::LEFTSIDE:
		camPos_.z -= camSpeed_;
		frameCount_ += 1;

		if (frameCount_ >= 300) {
			camPos_ = { 0.0f, 7.3f, 40.0f };
			camRot_ = { 0.14f, 91.1f, 0.0f };
			frameCount_ = 0;
			camPhase_ = CameraPhase::FRONT;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	case CameraPhase::FRONT:
		camPos_.z -= camSpeed_;
		frameCount_ += 1;

		if (frameCount_ >= 300) {
			camPos_ = { 30.0f, 7.3f, 30.0f };
			camRot_ = { 0.14f, -90.0f, 0.0f };
			frameCount_ = 0;
			camPhase_ = CameraPhase::RIGHTSIDE;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	case CameraPhase::RIGHTSIDE:
		camPos_.z -= camSpeed_;
		frameCount_ += 1;

		if (frameCount_ >= 300) {
			camPos_ = { 0.0f, 7.3f, -50.0f };
			camRot_ = { 0.14f, 0.0f, 0.0f };
			frameCount_ = 0;
			camPhase_ = CameraPhase::BACK;
		}

		camera->SetTranslate(camPos_);
		camera->SetRotate(camRot_);

		break;
	}
    Editor::GetInstance()->Clear();
}
