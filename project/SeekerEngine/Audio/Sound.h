#pragma once
#include <Windows.h>
#include <wrl.h>
#include <xAudio2.h>
#include <fstream>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include "Logger.h"
#include <vector>
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

// チャンクヘッダ
struct ChunkHeader {
	char id[4]; // チャンク毎のID
	int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
};

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk; // "fmt"
	WAVEFORMATEX fmt; // 波形フォーマット
};

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	std::vector<BYTE> pBuffer;
};

struct VoiceContext {
	IXAudio2SourceVoice* pSource;
	float initialVolume;
};

class Sound
{
public:
	Sound();

	void Init();
	void Shutdown();

	void Update();

	SoundData SoundLoad(const char* filename);
	void SoundUnload(SoundData* soundData);

	void PlayBGM(const SoundData& data, bool loop = false, float volume = 1.0f);
	void PlaySE(const SoundData& data, bool loop = false, float volume = 1.0f);
	void StopBGM();
	void StopSE();
	void RestartBGM();
	void SetVolumeBGM(float volume);
	void SetVolumeSE(float volume);
	void SetVolumeMaster(float volume);

	IXAudio2SourceVoice* GetBGMVoice() const { return bgmVoice_; }
	std::vector<VoiceContext> GetSEVoices() const { return seVoices_; }
	float GetCurrentBGMVolume() const { return currentBGMVolume_; }
	float GetCurrentSEVolume() const { return currentSEVolume_; }
	float GetCurrentMasterVolume() const { return currentMasterVolume_; }

private:
	// 内部用
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	bool mfStarted_ = false;
	static std::wstring ToWide(const char* utf8);
	static std::string  ToLowerExt(const std::string& path);
	SoundData SoundLoadMP3(const wchar_t* wpath);
	SoundData SoundLoadWave(const char* filename);

	IXAudio2MasteringVoice* masterVoice_ = nullptr;
	IXAudio2SourceVoice* bgmVoice_ = nullptr;
	std::vector<VoiceContext> seVoices_;
	
	bool isLooping_ = false;
	float currentVolume_ = 1.0f;
	float currentBGMVolume_ = 1.0f;
	float currentSEVolume_ = 1.0f;
	float currentMasterVolume_ = 1.0f;
	SoundData currentData_ = {};
};

