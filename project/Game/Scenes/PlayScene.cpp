#include "PlayScene.h"
#include "SceneIncludes.h"
#include "MathFunc.h"

void PlayScene::Init()
{
    Logger::Write("現在シーンPlayScene");
	//===========================
	// サウンド
	//===========================
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Load("bgm_play", "bgm_play.wav");
	soundMgr->PlayBGM("bgm_play");

	//===========================
	// カメラマネージャー
	//===========================
	auto camMgr = CameraManager::GetInstance();
	auto entityCommon = Entity3DCommon::GetInstance();
	entityCommon->SetCameraManager(camMgr);
	entityCommon->SetDebugCamera(camMgr->GetDebugCamera());
	entityCommon->SetDefaultCamera(camMgr->GetActiveCamera());

	// カメラの初期位置設定
	auto camera = camMgr->GetCamera("MainCamera");
	Vector3 camPos = { 0.0f, 0.0f, -105.0f };
	Vector3 camRot = { 0.0f, 0.0f, 0.0f };
	camera->SetTranslate(camPos);
	camera->SetRotate(camRot);

	//===========================
	// テクスチャ
	//===========================
	auto texMgr = TextureManager::GetInstance();
	texReticle_ = texMgr->Load("resources/sprites/ui_reticle.png");
	texUiPlayOperate_ = texMgr->Load("resources/sprites/ui_play_operate.png");

	//===========================
	// モデルロード
	//===========================
	auto modelMgr = ModelManager::GetInstance();
	modelMgr->LoadModel("bullet.obj");
	modelMgr->LoadModel("enemy.obj");

	//===========================
	// スプライト
	//===========================
	sprReticle_ = std::make_unique<Sprite>();
	sprReticle_->Init();
	sprReticle_->Create(texReticle_, { 0.0f, 0.0f }, Color::WHITE);
	sprReticle_->SetAnchorPoint({ 0.5f, 0.5f });
	Editor::GetInstance()->RegisterSprite("sprReticle", sprReticle_.get());

	sprUiPlayOperate_ = std::make_unique<Sprite>();
	sprUiPlayOperate_->Init();
	sprUiPlayOperate_->Create(texUiPlayOperate_, { 845.0f, 668.0f }, Color::WHITE);
	Editor::GetInstance()->RegisterSprite("sprUiPlayOperate", sprUiPlayOperate_.get());

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
	player_ = std::make_shared<Player>();
	player_->Init({ 0.0f, 0.0f, -10.0f });

	enemyMgr_ = std::make_unique<EnemyManager>();
	enemyMgr_->Init(player_);

	fade_.Init();
	fade_.Start(Fade::Status::FadeIn, 1.0f);
    ImGuiManager::GetInstance()->LoadScenesJson();
}

void PlayScene::Update()
{
	auto camMgr = CameraManager::GetInstance();
    /*-- 更新処理 --*/
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
	camMgr->Update();

	// モデルの更新処理
	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelSkydome_->SetCamera(camera);
	}

#ifdef _DEBUG
	if (Input::GetInstance()->IsPress(DIK_1)) {
		isGameStop_ = !isGameStop_;
	}

	if (isGameStop_) {
		enemyMgr_->SetIsMoveStop(true);
	} else {
		enemyMgr_->SetIsMoveStop(false);
	}
#endif

	player_->Update();
	modelSkydome_->Update();
	enemyMgr_->Update();

	// スプライトの更新処理
	UpdateReticle();
	sprReticle_->Update();
	sprUiPlayOperate_->Update();
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

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
	// Model
	player_->Draw();
	enemyMgr_->Draw();
	modelSkydome_->Draw();

	// Sprite
	sprReticle_->Draw();
	sprUiPlayOperate_->Draw();
	fade_.Draw();

    ImGuiManager::GetInstance()->Draw();
}

void PlayScene::Shutdown()
{
	auto soundMgr = SoundManager::GetInstance();
	soundMgr->Unload("bgm_play");
    Editor::GetInstance()->Clear();
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
	if (CameraManager::GetInstance()->GetIsDebug()) {
		return;
	}

	Camera* camera = CameraManager::GetInstance()->GetCamera("MainCamera");

	if (!camera) {
		return;
	}

	Transform playerTransform = player_->GetModel()->GetTransform();
	Transform cameraTransform = CameraManager::GetInstance()->GetActiveCamera()->GetTransform();

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