#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new SceneManager;
	}
	return instance_;
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
}

SceneManager::SceneManager()
{
	scene_ = std::make_unique<TitleScene>();
	nextScene_ = std::move(scene_);
}

SceneManager::~SceneManager()
{
	scene_->Shutdown();
	scene_.reset();
}
