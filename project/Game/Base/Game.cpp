#include "Game.h"
#include "SceneFactory.h"

void Game::Init()
{
	SEFramework::Init();
	SetSceneFactory(std::make_unique<SceneFactory>());
}

void Game::Shutdown()
{
	SEFramework::Shutdown();
}

void Game::Update()
{
	SEFramework::Update();
}

void Game::Draw()
{
	
}
