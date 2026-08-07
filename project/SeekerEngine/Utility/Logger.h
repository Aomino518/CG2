#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <mutex>

class Logger {
public:
	enum class LogLevel {
		Debug,
		Info,
		Warning,
		Error
	};

	static void Init(); // 初期化
	static void Shutdown(); // 終了処理
	static void Write(const std::string& msg); // 書き込み

	static void Write(LogLevel level, const std::string& msg);
	static void SetLevel(LogLevel level);

	static std::vector<std::string> GetHistory();
	static void ClearHistory();

private:
	static std::ofstream stream_; // 出力先のファイルストリーム
	static LogLevel currentLevel_;

	static constexpr int kMaxLogFiles = 10;

	static void RemoveOldLogs();
	static std::string GetDate();

	static std::vector<std::string> logHistory_;
	static std::mutex mutex_;
	static constexpr size_t kMaxHistory_ = 1000;
};
