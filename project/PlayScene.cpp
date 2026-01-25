#include "PlayScene.h"

void PlayScene::Init()
{
   //===========================
   // Sound
   //===========================

    // ※シーンをループすると毎回ロードされて使用メモリが増加する問題が発生中
    //   SoundManagerの実装を検討中
    /*bgm = std::make_unique<Sound>();
    se = std::make_unique<Sound>();

    sHAudio1 = bgm->SoundLoad("resources/c21.mp3");
    sHAudio2 = bgm->SoundLoad("resources/koharubiyori.mp3");
    sHAudio3 = se->SoundLoad("resources/gold.mp3");
    sHAudio4 = se->SoundLoad("resources/se_itemget.wav");*/

    //===========================
    // Camera
    //===========================
    cameraManager = std::make_unique<CameraManager>();
    cameraManager->Init();
    cameraManager->CreateCamera("EntranceCamera");
    Entity3DCommon::GetInstance()->SetCameraManager(cameraManager.get());
    Entity3DCommon::GetInstance()->SetDebugCamera(cameraManager->GetDebugCamera());

    //===========================
    // Sprite
    //===========================
    sprite = std::make_unique<Sprite>();
    uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/uvChecker.png");
    sprite->Create(tHChecker, { 0.0f, 0.0f }, Color::WHITE);
    sprite->SetRotation(0.0f);

    //===========================
    // Model
    //===========================
    entity = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball");
    entity->Init();
    entity->SetModel("ball");
    entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

    modelTerrain = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("terrain");
    modelTerrain->Init();
    modelTerrain->SetModel("terrain");
    modelTerrain->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

    //===========================
    // Particle
    //===========================

    // ※シーンをループすると毎回登録される問題が発生中
    //ParticleManager::GetInstance()->CreateParticleGroup("Smoke", tHChecker);
    //emitter_ = std::make_unique<ParticleEmitter>("Smoke", 10, 0.1f);
	
}

void PlayScene::Update()
{
    /*-- 更新処理 --*/
    /*if (Input::GetInstance()->IsPressed(DIK_S)) {
        bgm->SoundPlay(sHAudio1, false);
    }

    if (Input::GetInstance()->IsPressed(DIK_N)) {
        bgm->SoundPlay(sHAudio2, false);
    }

    if (Input::GetInstance()->IsPressed(DIK_M)) {
        se->SoundPlay(sHAudio3, false);
    }

    if (Input::GetInstance()->IsPressed(DIK_V)) {
        se->SoundPlay(sHAudio4, false);
    }

    if (Input::GetInstance()->IsPressed(DIK_B)) {
        bgm->SoundStop();
        se->SoundStop();
    }*/

    if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
        SceneManager::GetInstance()->SetNextScene(std::make_unique<TitleScene>());
    }

    cameraManager->Update();

    //emitter_->Update();
	//ParticleManager::GetInstance()->Update(cameraManager.get());

    sprite->Update();

    entity->SetCamera(cameraManager->GetActiveCamera());
    entity->Update();

    modelTerrain->SetCamera(cameraManager->GetActiveCamera());
    modelTerrain->Update();

    ImGuiManager::GetInstance()->BegineFrame();
    ImGuiManager::GetInstance()->BegineInspector();
    ImGuiManager::GetInstance()->CameraSetting(cameraManager.get());
    ImGuiManager::GetInstance()->SpriteSetting("uvChecker", sprite.get());
    ImGuiManager::GetInstance()->ModelSetting("ball", entity.get());
    ImGuiManager::GetInstance()->ModelSetting("terrain", modelTerrain.get());
    //ImGuiManager::GetInstance()->ParticleSetting("Smoke", emitter_.get());
    ImGuiManager::GetInstance()->EndInspector();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->LightSetting();
    ImGuiManager::GetInstance()->EndFrame();
}

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
    Entity3DCommon::GetInstance()->DrawCommon();
    entity->Draw();
    modelTerrain->Draw();
   
    //ParticleManager::GetInstance()->Draw();

    SpriteCommon::GetInstance()->DrawCommon();
    sprite->Draw();

    ImGuiManager::GetInstance()->Draw();
}

void PlayScene::Shutdown()
{
}
