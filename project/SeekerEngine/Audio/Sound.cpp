#include "Sound.h"
#include <assert.h>
#include <algorithm>

Sound::Sound() {
}

void Sound::Init() {
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr)) {
		Logger::Write(Logger::LogLevel::Error, "XAudio2Create Failed");
		return;
	}

	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
	if (FAILED(hr)) {
		Logger::Write(Logger::LogLevel::Error, "CreateMasteringVoice Failed");
		return;
	}

	hr = MFStartup(MF_VERSION, 0);
	if (SUCCEEDED(hr)) {
		mfStarted_ = true;
	}
}

void Sound::Shutdown() {
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
	Logger::Write("Sound Shutdown");
}

/// <summary>
/// 音声読み込み関数
/// </summary>
/// <param name="filename">ファイル名</param>
/// <returns>サウンドハンドル</returns>
SoundData Sound::SoundLoad(const char* filename)
{
	Logger::Write("Soundロード開始");
	std::string path(filename ? filename : "");
	std::string ext = ToLowerExt(path);

	SoundData soundData = {};

	Logger::Write("Soundがwavかmp3か判定開始");
	if (ext == ".wav") {
		Logger::Write("Soundはwav");
		soundData = SoundLoadWave(filename);
	}
	if (ext == ".mp3") {
		Logger::Write("Soundはmp3");
		std::wstring w = ToWide(filename);
		soundData = SoundLoadMP3(w.c_str());
	}

	return soundData;
}

// 音声データ解放
void Sound::SoundUnload(SoundData* soundData) {
	soundData->pBuffer.clear();
	soundData->pBuffer.shrink_to_fit();
	soundData->wfex = {};
}

void Sound::PlayBGM(const SoundData& data, bool loop, float volume)
{
	StopBGM();

	isLooping_ = loop;
	currentData_ = data;

	xAudio2_->CreateSourceVoice(
		&bgmVoice_,
		&data.wfex
	);

	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = data.pBuffer.data();
	buffer.AudioBytes = (UINT32)data.pBuffer.size();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	if (loop) {
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	float finalVolume = currentBGMVolume_ * currentMasterVolume_ * volume;
	bgmVoice_->SetVolume(finalVolume);

	bgmVoice_->SubmitSourceBuffer(&buffer);
	bgmVoice_->Start();
}

void Sound::PlaySE(const SoundData& data, bool loop, float volume)
{
	IXAudio2SourceVoice* seVoice = nullptr;

	xAudio2_->CreateSourceVoice(
		&seVoice,
		&data.wfex
	);

	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = data.pBuffer.data();
	buffer.AudioBytes = (UINT32)data.pBuffer.size();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	if (loop) {
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	float finalVolume = currentSEVolume_ * currentMasterVolume_ * volume;
	seVoice->SetVolume(finalVolume);

	seVoice->SubmitSourceBuffer(&buffer);
	seVoice->Start();

	seVoices_.push_back({ seVoice, volume });
}

void Sound::StopBGM() {
	if (bgmVoice_) {
		bgmVoice_->Stop();
		bgmVoice_->FlushSourceBuffers();
		bgmVoice_->DestroyVoice();
		bgmVoice_ = nullptr;
	}
}

void Sound::StopSE()
{
	for (auto& v : seVoices_) {
		v.pSource->Stop();
		v.pSource->FlushSourceBuffers();
		v.pSource->DestroyVoice();
	}
	seVoices_.clear();
}

void Sound::RestartBGM()
{
	if (!currentData_.pBuffer.empty()) {
		return;
	}

	PlayBGM(currentData_, isLooping_);
}

void Sound::SetVolumeBGM(float volume)
{
	currentBGMVolume_ = std::clamp(volume, 0.0f, 1.0f);
	if (bgmVoice_) {
		bgmVoice_->SetVolume(currentBGMVolume_);
	}
}

void Sound::SetVolumeSE(float volume)
{
	currentSEVolume_ = std::clamp(volume, 0.0f, 1.0f);
	for (auto& v : seVoices_) {
		float finalVolume = currentSEVolume_ * currentMasterVolume_ * v.initialVolume;
		v.pSource->SetVolume(finalVolume);
	}
}

void Sound::SetVolumeMaster(float volume)
{
	currentMasterVolume_ = std::clamp(volume, 0.0f, 1.0f);
	float bgmVolume = currentBGMVolume_ * currentMasterVolume_;
	if (bgmVoice_) {
		bgmVoice_->SetVolume(bgmVolume);
	}

	for (auto& v : seVoices_) {
		float finalVolume = currentSEVolume_ * currentMasterVolume_ * v.initialVolume;
		v.pSource->SetVolume(finalVolume);
	}
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
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	return ext;
}

// wave読み込み
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
	while (true) {
		file.read((char*)&format.chunk, sizeof(ChunkHeader));
		if (file.eof()) assert(0 && "fmtチャンクが見つかりません");

		if (strncmp(format.chunk.id, "fmt ", 4) == 0) {
			break;
		}

		// fmt以外なら中身を飛ばして次へ
		file.seekg(format.chunk.size, std::ios_base::cur);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	while (true) {
		file.read((char*)&data, sizeof(data));
		if (file.eof()) {
			assert(0 && "dataチャンクが見つかりませんでした");
		}

		// "data" チャンクを見つけたらループを抜ける
		if (strncmp(data.id, "data", 4) == 0) {
			break;
		}

		// それ以外のチャンク（"LIST" や "JUNK" など）はサイズ分だけ読み飛ばす
		file.seekg(data.size, std::ios_base::cur);
	}

	/*--読み込んだ音声データをリターン--*/
	// returnするための音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer.resize(data.size);
	file.read(reinterpret_cast<char*>(soundData.pBuffer.data()), data.size);

	file.close();
	return soundData;
}

// mp3読み込み
SoundData Sound::SoundLoadMP3(const wchar_t* wpath)
{
	Logger::Write("SoundLoadMp3開始");
	SoundData soundData = {};
	if (!mfStarted_) {
		Logger::Write(Logger::LogLevel::Error, "Media Foundation is not started!");
		return soundData;
	}

	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };
	MFCreateSourceReaderFromURL(wpath, nullptr, &pMFSourceReader);
	
	// ストリーム選択
	HRESULT hr = pMFSourceReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);

	if (FAILED(hr)) {
		return soundData;
	}
	
	hr = pMFSourceReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), TRUE);

	if (FAILED(hr)) {
		return soundData;
	}
	
	// ネイティブ型からch/rateを取っておく
	Microsoft::WRL::ComPtr<IMFMediaType> pMFMediaType{ nullptr };
	UINT32 ch = 2, rate = 48000;
	if (SUCCEEDED(pMFSourceReader->GetNativeMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, &pMFMediaType))) {
		UINT32 v = 0;
		if (SUCCEEDED(pMFMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &v))) {
			ch = v;
		}

		if (SUCCEEDED(pMFMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &v))) {
			rate = v;
		}
	}

	hr = MFCreateMediaType(&pMFMediaType);

	if (FAILED(hr)) {
		Logger::Write(Logger::LogLevel::Error, "MFCreateMediaTypeエラー");
		return soundData;
	}

	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, ch);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, rate);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (ch * 16) / 8);
	pMFMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, rate * (ch * 16) / 8);

	hr = pMFSourceReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType.Get());
	
	if (FAILED(hr)) {
		Logger::Write(Logger::LogLevel::Error, "SetCurrentMediaTypeエラー");
		return soundData;
	}
	
	Microsoft::WRL::ComPtr<IMFMediaType> finalType;
	hr = pMFSourceReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &finalType);
	
	if (FAILED(hr)) {
		Logger::Write(Logger::LogLevel::Error, "GetCurrentMediaTypeエラー");
		return soundData;
	}
	
	WAVEFORMATEX* waveFormat{ nullptr };
	hr = MFCreateWaveFormatExFromMFMediaType(finalType.Get(), &waveFormat, nullptr);

	if (FAILED(hr) || waveFormat == nullptr) {
		Logger::Write(Logger::LogLevel::Error, "waveFormatがnullptrかMFCreateWaveFormatExFromMFMediaTypeエラー");
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
		DWORD cbCurrentLength = 0, maxLength = 0;
		pMFMediaBuffer->Lock(&pBuffer, &maxLength, &cbCurrentLength);

		if (cbCurrentLength) {
			size_t old = mediaData.size();
			mediaData.resize(old + cbCurrentLength);
			std::memcpy(mediaData.data() + old, pBuffer, cbCurrentLength);
		}

		pMFMediaBuffer->Unlock();
	}

	soundData.wfex = *waveFormat;
	soundData.pBuffer = std::move(mediaData);
	
	CoTaskMemFree(waveFormat);

	return soundData;
}