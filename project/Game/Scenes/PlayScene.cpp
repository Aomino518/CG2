#include "PlayScene.h"

void PlayScene::Init()
{
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("bgm_play", "resources/bgm_play.wav");
	soundMgr->PlayBGM("bgm_play");

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
	texReticle_ = texMgr->Load("resources/ui_reticle.png");
	texUiPlayOperate_ = texMgr->Load("resources/ui_play_operate.png");

	//===========================
	// モデルロード
	//===========================
	auto modelMgr = ModelManager::GetInstance();
	modelMgr->LoadModel("bullet");
	modelMgr->LoadModel("enemy");

	//===========================
	// スプライト
	//===========================
	sprReticle_ = std::make_unique<Sprite>();
	sprReticle_->Init();
	sprReticle_->Create(texReticle_, { 0.0f, 0.0f }, Color::WHITE);
	sprReticle_->SetAnchorPoint({ 0.5f, 0.5f });

	sprUiPlayOperate_ = std::make_unique<Sprite>();
	sprUiPlayOperate_->Init();
	sprUiPlayOperate_->Create(texUiPlayOperate_, { 845.0f, 668.0f }, Color::WHITE);

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
	player_ = std::make_shared<Player>();
	player_->Init({0.0f, 0.0f, -10.0f});

	enemyMgr_ = std::make_unique<EnemyManager>();
	enemyMgr_->Init(player_);

	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
}

void PlayScene::Update()
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
		UpdatePlay();
		break;
	case ScenePhase::FADEOUT:
		if (fade_.IsFinished()) {
			if (isClear) {
				soundMgr->StopBGM();
				SceneManager::GetInstance()->ChangeScene("CLEAR");
			} else if (isGameOver) {
				soundMgr->StopBGM();
				SceneManager::GetInstance()->ChangeScene("GAMEOVER");
			}
		}
		break;
	}

	// カメラの更新処理
	camMgr_->Update();

	// モデルの更新処理
	player_->Update();
	modelSkydome_->Update();
	enemyMgr_->Update();

	// スプライトの更新処理
	UpdateReticle();
	sprReticle_->Update();
	sprUiPlayOperate_->Update();
	fade_.Update();

	UpdateImGui();
}

void PlayScene::Draw()
{
	/*-- 描画処理 --*/
	Entity3DCommon::GetInstance()->DrawCommon();
	player_->Draw();
	enemyMgr_->Draw();
	modelSkydome_->Draw();

	SpriteCommon::GetInstance()->DrawCommon();
	sprReticle_->Draw();
	sprUiPlayOperate_->Draw();
	fade_.Draw();

	ImGuiManager::GetInstance()->Draw();
}

void PlayScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Unload("bgm_play");
}

void PlayScene::UpdatePlay()
{
	if (enemyMgr_->GetIsFinished()) {
		isClear = true;
		fade_.Start(Fade::Status::FadeOut, 1.0f);
		phase_ = ScenePhase::FADEOUT;
	}

	if (!player_->GetIsAlive()) {
		isGameOver = true;
		fade_.Start(Fade::Status::FadeOut, 1.0f);
		phase_ = ScenePhase::FADEOUT;
	}
}

void PlayScene::UpdateReticle()
{
	Transform playerTransform = player_->GetModel()->GetTransform();
	Transform cameraTransform = camMgr_->GetActiveCamera()->GetTransform();

	// 前方向取得
	Vector3 forward = GetForwardFromTransform(playerTransform.rotate);

	// 3Dレティクル位置
	Vector3 reticleWorldPos = playerTransform.translate + forward * 100.0f;

	// View / Projection
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);

	Matrix4x4 projectionMatrix =
		MakePerspectiveFovMatrix(
			0.45f,
			float(Graphics::GetWidth()) / float(Graphics::GetHeight()),
			0.1f,
			100.0f);

	// 3Dから2D
	Vector3 clip = TransformToVector3(reticleWorldPos, viewMatrix * projectionMatrix);

	Vector2 screen;
	screen.x = (clip.x + 1.0f) * 0.5f * Graphics::GetWidth();
	screen.y = (1.0f - clip.y) * 0.5f * Graphics::GetHeight();

	// UI反映
	sprReticle_->SetPosition(screen);
}

void PlayScene::UpdateImGui() {
	auto imguiMgr = ImGuiManager::GetInstance();
	imguiMgr->BegineFrame();
	imguiMgr->BegineInspector();
	imguiMgr->SpriteSetting("UiPlayOperate", sprUiPlayOperate_.get());
	imguiMgr->ModelSetting("StarSkyDome", modelSkydome_.get());
	imguiMgr->EndInspector();
	imguiMgr->CameraSetting(camMgr_.get());
	imguiMgr->Stats();
	imguiMgr->EndFrame();
}