#include "Sound.h"
#include <vector>
#include <assert.h>

bool Sound::Init()
{
	HRESULT result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	result = xAudio2_->CreateMasteringVoice(&masterVoice_);

	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	if (SUCCEEDED(result)) {
		mfStarted_ = true;
	}

	return true;
}

void Sound::Shutdown()
{
	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}

	// XAudio2解放
	xAudio2_.Reset();

	if (mfStarted_) {
		MFShutdown();
		mfStarted_ = false;
	}
}

SoundData Sound::SoundLoadWave(const char* filename) {
	/*--ファイルオープン--*/
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリ―モードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	/*--.wavデータ読み込み--*/
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// WAVEファイルを閉じる
	file.close();

	/*--読み込んだ音声データをリターン--*/
	// returnするための音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

SoundData Sound::SoundLoadMP3(const wchar_t* wpath)
{
	SoundData soundData = {};
	if (!mfStarted_) {
		return soundData;
	}

	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };
	MFCreateSourceReaderFromURL(wpath, NULL, &pMFSourceReader);

	Microsoft::WRL::ComPtr<IMFMediaType> pMFMediaType{ nullptr };
	HRESULT hr = MFCreateMediaType(&pMFMediaType);

	if (FAILED(hr)) {
		return soundData;
	}

	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	hr = pMFSourceReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType.Get());
	
	if (FAILED(hr)) {
		return soundData;
	}

	Microsoft::WRL::ComPtr<IMFMediaType> finalType;
	hr = pMFSourceReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &finalType);
	
	if (FAILED(hr)) {
		return soundData;
	}

	std::vector<BYTE> mediaData;

	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> pMFSample{ nullptr };
		DWORD dwStreamsFlags{ 0 };
		pMFSourceReader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &dwStreamsFlags, nullptr, &pMFSample);

		if (dwStreamsFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		Microsoft::WRL::ComPtr<IMFMediaBuffer> pMFMediaBuffer{ nullptr };
		pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

		BYTE* pBuffer{ nullptr };
		DWORD cbCurrentLength{ 0 };
		pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

		mediaData.resize(static_cast<size_t>(mediaData.size()) + static_cast<size_t>(cbCurrentLength));
		memcpy(mediaData.data() + mediaData.size() - static_cast<size_t>(cbCurrentLength), pBuffer, static_cast<size_t>(cbCurrentLength));

		pMFMediaBuffer->Unlock();
	}

	WAVEFORMATEX* waveFormat{ nullptr };
	hr = MFCreateWaveFormatExFromMFMediaType(finalType.Get(), &waveFormat, nullptr);
	
	if (FAILED(hr) || waveFormat == nullptr) {
		return soundData;
	}

	BYTE* heap = new BYTE[mediaData.size()];
	memcpy(heap, mediaData.data(), mediaData.size());

	soundData.wfex = *waveFormat;
	soundData.pBuffer = heap;
	soundData.bufferSize = static_cast<unsigned int>(mediaData.size());

	CoTaskMemFree(waveFormat);

	return soundData;
}

SoundData Sound::SoundLoad(const char* filename)
{
	std::string path(filename ? filename : "");
	std::string ext = ToLowerExt(path);

	SoundData soundData = {};

	if (ext == ".wav") {
		soundData = SoundLoadWave(filename);
	}
	if (ext == ".mp3") {
		std::wstring w = ToWide(filename);
		soundData = SoundLoadMP3(w.c_str());
	}

	return soundData;
}

// 音声再生
void Sound::SoundPlayWave(const SoundData& soundData) {
	HRESULT result;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

// 音声データ解放
void Sound::SoundUnload(SoundData* soundData) {
	// バッファのメモリ解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

std::wstring Sound::ToWide(const char* utf8) {
	if (!utf8) {
		return L"";
	}

	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
	std::wstring w(wlen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w.data(), wlen);
	if (!w.empty() && w.back() == L'\0') {
		w.pop_back();
	}

	return w;
}

std::string Sound::ToLowerExt(const std::string& path) {
	auto pos = path.find_last_of('.');
	std::string ext = (pos == std::string::npos) ? "" : path.substr(pos);
	return ext;
}