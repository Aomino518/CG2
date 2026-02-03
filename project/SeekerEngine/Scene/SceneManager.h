#pragma once
#include "SeekerEngine.h"
#include "BaseScene.h"
#include "AbstractSceneFactory.h"

class SceneManager
{
public:
	static SceneManager* GetInstance();

	void SetNextScene(std::unique_ptr<BaseScene> nextScene) { nextScene_ = std::move(nextScene); }

	void Update();

	void Draw();

	void Shutdown();

	void ChangeScene(const std::string& sceneName);

	// setter
	void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory) { 
		this->sceneFactory_ = std::move(sceneFactory);
	}


private:
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	std::unique_ptr<BaseScene> scene_;
	std::unique_ptr<BaseScene> nextScene_;
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
};

