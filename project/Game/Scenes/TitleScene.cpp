#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Init()
{
    //===========================
    // カメラマネージャー
    //===========================
    camMgr_ = std::make_unique<CameraManager>();
    camMgr_->Init();
    auto entityCommon = Entity3DCommon::GetInstance();
    entityCommon->SetCameraManager(camMgr_.get());
    entityCommon->SetDebugCamera(camMgr_->GetDebugCamera());
    entityCommon->SetDefaultCamera(camMgr_->GetActiveCamera());

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
}

void TitleScene::Update()
{
    if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
       SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    camMgr_->Update();

    // モデルの更新処理
    modelPlayer_->Update();

    // スプライトの更新処理
    sprTitleLogo_->Update();
    sprUiPressSpace_->UpdateColorBlink(Color::WHITE, Color::YELLOW);
    sprUiPressSpace_->Update();

    // ImGuiの更新処理
    auto imguiMgr = ImGuiManager::GetInstance();
    imguiMgr->BegineFrame();
    imguiMgr->BegineInspector();
    imguiMgr->SpriteSetting("TitleLogo", sprTitleLogo_.get());
    imguiMgr->SpriteSetting("UiPressSpace", sprUiPressSpace_.get());
    imguiMgr->ModelSetting("Player", modelPlayer_.get());
    imguiMgr->EndInspector();
    imguiMgr->CameraSetting(camMgr_.get());
    imguiMgr->Stats();
    imguiMgr->EndFrame();
}

void TitleScene::Draw()
{
    Entity3DCommon::GetInstance()->DrawCommon();
    modelPlayer_->Draw();

    SpriteCommon::GetInstance()->DrawCommon();
    sprTitleLogo_->Draw();
    sprUiPressSpace_->Draw();

    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
}
