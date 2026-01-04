#include "Game.h"

void Game::Init()
{
	SEFramework::Init();

    //===========================
    // ImGui
    //===========================
    imgui.Init(engine_.GetApp(), engine_.GetGraphics());

}

void Game::Shutdown()
{
	imgui.Shutdown();

	SEFramework::Shutdown();
}

void Game::Update()
{
	SEFramework::Update();

	imgui.BegineFrame();
	imgui.BegineInspector();
	imgui.EndInspector();
	imgui.Stats();
	imgui.EndFrame();
}

void Game::Draw()
{
	/*-- 描画処理 --*/
	

	imgui.Draw();
}
