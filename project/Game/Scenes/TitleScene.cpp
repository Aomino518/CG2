#include "TitleScene.h"
#include "SceneManager.h"

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
    Vector3 camPos = { 0.0f, 7.3f, -40.0f };
    Vector3 camRot = { 0.14f, 0.0f, 0.0f };
    camera->SetTranslate(camPos);
    camera->SetRotate(camRot);

    //===========================
    // テクスチャ
    //===========================
    auto texMgr = TextureManager::GetInstance();
    texTitleLogo_ = texMgr->Load("resources/spr_title_logo.png");
    texPressSpace_ = texMgr->Load("resources/ui_press_space.png");

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
    sprTitleLogo_->Create(texTitleLogo_, { 385.0f, 31.0f }, Color::WHITE, {500.0f, 290.0f});

    sprUiPressSpace_ = std::make_unique<Sprite>();
    sprUiPressSpace_->Init();
    sprUiPressSpace_->Create(texPressSpace_, { 415.0f, 575.0f }, Color::WHITE);

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
}

void TitleScene::Update()
{
    auto soundMgr = SoundManager::GetInstance();
    if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
       soundMgr->PlaySE("se_selected");
       soundMgr->StopBGM();
       SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    // カメラの更新処理
    camMgr_->Update();

    // モデルの更新処理
    modelPlayer_->Update();
    modelSkydome_->Update();

    // スプライトの更新処理
    sprTitleLogo_->Update();
    sprUiPressSpace_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
    sprUiPressSpace_->Update();

    // ImGuiの更新処理
    UpdateImGui();
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
    sprUiPressSpace_->Draw();

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
    imguiMgr->ModelSetting("Player", modelPlayer_.get());
    imguiMgr->ModelSetting("StarSkyDome", modelSkydome_.get());
    imguiMgr->EndInspector();
    imguiMgr->CameraSetting(camMgr_.get());
    imguiMgr->Stats();
    imguiMgr->EndFrame();
}
