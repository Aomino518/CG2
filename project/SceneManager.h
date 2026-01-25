#pragma once
#include "SeekerEngine.h"
#include "BaseScene.h"
#include "TitleScene.h"
#include "PlayScene.h"

class SceneManager
{
public:
	static SceneManager* GetInstance();

	void SetNextScene(std::unique_ptr<BaseScene> nextScene) { nextScene_ = std::move(nextScene); }

	void Update();

	void Draw();

	void Shutdown();

	SceneManager();
	~SceneManager();

private:
	static SceneManager* instance_;

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	std::unique_ptr<BaseScene> scene_;
	std::unique_ptr<BaseScene> nextScene_;
};

