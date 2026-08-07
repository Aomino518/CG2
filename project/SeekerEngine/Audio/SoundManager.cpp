#include "SoundManager.h"

SoundManager* SoundManager::GetInstance() {
	static SoundManager instance;
	return &instance;
}

void SoundManager::Init() {
	sound_.Init();
}

void SoundManager::Shutdown() {
	sound_.StopBGM();
	sound_.StopSE();
	for (auto& [name, data] : soundDatas_) {
		sound_.SoundUnload(&data);
	}

	sound_.Shutdown();
	soundDatas_.clear();
	currentBGM_ = nullptr;
	Logger::Write("SoundManager Shutdown");
}

void SoundManager::Update() {
	sound_.Update();
}

void SoundManager::Load(const std::string& name, const std::string& filepath) {
	if (soundDatas_.contains(name)) {
		return;
	}

	std::string soundFilePath = "resources/sounds/" + filepath;

	soundDatas_[name] = sound_.SoundLoad(soundFilePath.c_str());
}

void SoundManager::PlaySE(const std::string& name, float volume) {
	auto it = soundDatas_.find(name);
	if (it == soundDatas_.end()) {
		return;
	}

	sound_.PlaySE(it->second, false, volume);
}

void SoundManager::PlayBGM(const std::string& name, bool loop, float volume) {
	auto it = soundDatas_.find(name);
	if (it == soundDatas_.end()) {
		return;
	}

	currentBGM_ = &it->second;
	sound_.PlayBGM(*currentBGM_, loop, volume);
}

void SoundManager::StopSE() {
	sound_.StopSE();
}

void SoundManager::StopBGM() {
	sound_.StopBGM();
}

void SoundManager::Unload(const std::string& name) {
	auto it = soundDatas_.find(name);
	if (it == soundDatas_.end()) {
		return;
	}

	if (currentBGM_ && currentBGM_ == &it->second) {
		currentBGM_ = nullptr;
	}

	sound_.SoundUnload(&it->second);
	soundDatas_.erase(it);

}

void SoundManager::SetVolumeSE(float volume) {
	sound_.SetVolumeSE(volume);
}

void SoundManager::SetVolumeBGM(float volume) {
	sound_.SetVolumeBGM(volume);
}

void SoundManager::SetVolumeMaster(float volume) {
	sound_.SetVolumeMaster(volume);
}
