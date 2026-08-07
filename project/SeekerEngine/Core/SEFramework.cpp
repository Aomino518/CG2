#include "SEFramework.h"
#include "SceneManager.h"
#include "Application.h"

void SEFramework::Init()
{
}

void SEFramework::Shutdown()
{
}

void SEFramework::Update()
{
}

void SEFramework::Run()
{
	engine_.Init();

	Init();

	SceneManager::GetInstance()->SetSceneFactory(std::move(sceneFactory_));
	SceneManager::GetInstance()->ChangeScene("TITLE");

	// ウィンドウの×ボタンが押されるまでループ
	while (engine_.GetApp()->ProcessMessage()) {
		engine_.Update();
		Update();
		SceneManager::GetInstance()->Update();

		endRequst_ = SceneManager::GetInstance()->GetIsEndRequest();
		if (IsEndRequst()) {
			break;
		}

		/*-- 描画処理 --*/
		engine_.BegineFrame();
		Draw();
		SceneManager::GetInstance()->Draw();
		engine_.EndFrame();
	}

	Shutdown();
	engine_.Shutdown();
}

void SEFramework::SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory)
{
	sceneFactory_ = std::move(sceneFactory);
}
