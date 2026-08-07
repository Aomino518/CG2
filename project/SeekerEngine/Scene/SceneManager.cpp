#include "SceneManager.h"
#include "Logger.h"
#include <cassert>

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
}

void SceneManager::Update() {

	if (nextScene_) {
		// 旧シーン
		if (scene_) {
			scene_->Shutdown();
			scene_.reset();
		}

		// シーン切り替え
		scene_ = std::move(nextScene_);

		scene_->SetSceneManager(this);

		scene_->Init();
	}

	// 実行中シーンを更新する
	if (scene_) {
		scene_->Update();
	}
}

void SceneManager::Draw() {
	if (scene_) {
		scene_->Draw();
	}
}

void SceneManager::Shutdown() {
	scene_.reset();
	nextScene_.reset();
	sceneFactory_.reset();
	Logger::Write("SceneManager Shutdown");
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);

	nextScene_ = sceneFactory_->CreateScene(sceneName);
}

const char* SceneManager::GetCurrentSceneName() const
{
	if (!scene_) {
		return "Unknown";
	}
	return scene_->GetSceneName();
}
