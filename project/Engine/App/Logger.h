#pragma once
#include <string.h>
#include <fstream>

class Logger {
public:
	static void Init(); // 初期化
	static void Shutdown(); // 終了処理
	static void Write(const std::string& msg); // 書き込み

private:
	static std::ofstream stream_; // 出力先のファイルストリーム
};
