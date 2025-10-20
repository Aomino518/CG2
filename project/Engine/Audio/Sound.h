#pragma once
#include <Windows.h>
#include <wrl.h>
#include <xAudio2.h>
#include <fstream>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
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
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};

class Sound
{
public:
	bool Init();
	void Shutdown();
	SoundData SoundLoadWave(const char* filename);
	SoundData SoundLoadMP3(const wchar_t* wpath);
	SoundData SoundLoad(const char* filename);
	void SoundPlayWave(const SoundData& soundData);
	void SoundUnload(SoundData* soundData);

private:
	static std::wstring ToWide(const char* utf8);
	static std::string  ToLowerExt(const std::string& path);

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;

	bool mfStarted_ = false;
};

