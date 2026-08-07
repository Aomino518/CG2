#include "Logger.h"
#include <filesystem>
#include <chrono>
#include <format>
#include <Windows.h>

std::ofstream Logger::stream_;
Logger::LogLevel Logger::currentLevel_ = Logger::LogLevel::Info;

std::vector<std::string> Logger::logHistory_;
std::mutex Logger::mutex_;

void Logger::Init()
{
	// ログのディレクトリを用意
	std::filesystem::create_directory("logs");
	// ログ数の上限を超えていないか確認
	RemoveOldLogs();
	// 現在時刻を取得　(UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間 (PCの設定時間)　に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	stream_.open(logFilePath, std::ios::out);

#ifdef _DEBUG
	currentLevel_ = LogLevel::Debug;
#else
	currentLevel_ = LogLevel::Info;
#endif

	std::lock_guard<std::mutex> lock(mutex_);
	logHistory_.clear();
}

void Logger::Shutdown()
{
	if (stream_.is_open()) {
		stream_.close();
	}
}

void Logger::Write(const std::string& msg)
{
#ifdef _DEBUG
	Write(LogLevel::Debug, msg);
#else 
	Write(LogLevel::Info, msg);
#endif
}

void Logger::Write(LogLevel level, const std::string& msg)
{
	if (level < currentLevel_) {
		return;
	}

	const char* prefix = "";
	switch (level) {
	case LogLevel::Debug: 
		prefix = "[DEBUG] "; 
		break;
	case LogLevel::Info:  
		prefix = "[INFO] "; 
		break;
	case LogLevel::Warning:  
		prefix = "[WARNING] "; 
		break;
	case LogLevel::Error: 
		prefix = "[ERROR] "; 
		break;
	}

	std::string line = "[" + GetDate() + "]" + std::string(prefix) + msg;

	// メモリに履歴を保存
	std::lock_guard<std::mutex> lock(mutex_);
	logHistory_.push_back(line);

	if (logHistory_.size() > kMaxHistory_) {
		logHistory_.erase(logHistory_.begin());
	}

	if (stream_.is_open()) {
		stream_ << line << std::endl;
	}

	OutputDebugStringA((line + "\n").c_str());
}

void Logger::SetLevel(LogLevel level)
{
	currentLevel_ = level;
}

std::vector<std::string> Logger::GetHistory()
{
	std::lock_guard<std::mutex> lock(mutex_);
	return logHistory_;
}

void Logger::ClearHistory()
{
	std::lock_guard<std::mutex> lock(mutex_);
	logHistory_.clear();
}

void Logger::RemoveOldLogs()
{
	namespace fs = std::filesystem;

	std::vector<fs::directory_entry> logFiles;

	for (const auto& entry : fs::directory_iterator("logs")) {
		// 通常ファイルを指しているか
		if (!entry.is_regular_file()) {
			continue;
		}

		if (entry.path().extension() == ".log") {
			logFiles.push_back(entry);
		}
	}

	// ログの保存上限以内か
	if ((int)logFiles.size() < kMaxLogFiles) {
		return;
	}

	// 更新日時が古い順にソート
	std::sort(logFiles.begin(), logFiles.end(),
		[](const fs::directory_entry& a, const fs::directory_entry& b) {
			return fs::last_write_time(a) < fs::last_write_time(b);
		});

	// 上限を超えた分だけ削除
	int removeCount = (int)logFiles.size() - kMaxLogFiles + 1;
	for (int i = 0; i < removeCount; ++i) {
		fs::remove(logFiles[i]);
	}
}

std::string Logger::GetDate()
{
	// 現在時刻を取得　(UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間 (PCの設定時間)　に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%H:%M:%S}", localTime);
	return dateString;
}
