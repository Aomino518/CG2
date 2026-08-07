#pragma once
#include "Logger.h"
#include "Sound.h"
#include <string>
#include <unordered_map>

class SoundManager
{
public:
	static SoundManager* GetInstance();

	void Init();
	void Shutdown();

	void Update();

	void Load(const std::string& name, const std::string& filepath);

	void PlaySE(const std::string& name, float volume = 1.0f);
	void PlayBGM(const std::string& name, bool loop = true, float volume = 1.0f);
	void StopSE();
	void StopBGM();
	void SetVolumeSE(float volume);
	void SetVolumeBGM(float volume);
	void SetVolumeMaster(float volume);

	IXAudio2SourceVoice* GetBGMVoice() const { return sound_.GetBGMVoice(); }
	std::vector<VoiceContext> GetSEVoices() const { return sound_.GetSEVoices(); }
	float GetCurrentBGMVolume() const { return sound_.GetCurrentBGMVolume(); }
	float GetCurrentSEVolume() const { return sound_.GetCurrentSEVolume(); }
	float GetCurrentMasterVolume() const { return sound_.GetCurrentMasterVolume(); }

	void Unload(const std::string& name);

private:
	SoundManager() = default;
	~SoundManager() = default;
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

	Sound sound_;
	std::unordered_map<std::string, SoundData> soundDatas_;

	SoundData* currentBGM_ = nullptr;
};

