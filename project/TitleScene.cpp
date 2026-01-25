#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Init()
{
    //===========================
    // Sprite
    //===========================
    sprite = std::make_unique<Sprite>();
    uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/uvChecker.png");
    sprite->Create(tHChecker, { 0.0f, 0.0f }, Color::WHITE);
    sprite->SetRotation(0.0f);
}

void TitleScene::Update()
{
    if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
        SceneManager::GetInstance()->SetNextScene(std::make_unique<PlayScene>());
    }

    sprite->Update();

    ImGuiManager::GetInstance()->BegineFrame();
    ImGuiManager::GetInstance()->BegineInspector();
    ImGuiManager::GetInstance()->SpriteSetting("uvChecker", sprite.get());
    ImGuiManager::GetInstance()->EndInspector();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::Draw()
{
    SpriteCommon::GetInstance()->DrawCommon();
    sprite->Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
}
