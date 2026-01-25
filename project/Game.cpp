#include "Game.h"

void Game::Init()
{
	SEFramework::Init();
	//sceneManager_ = std::make_unique<SceneManager>();
}

void Game::Shutdown()
{
	SEFramework::Shutdown();
}

void Game::Update()
{
	SEFramework::Update();
	SceneManager::GetInstance()->Update();
}

void Game::Draw()
{
	SceneManager::GetInstance()->Draw();
}
