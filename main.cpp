#include "Application.h"
#include "Logger.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Logger::Init();
	Logger::Write("アプリ開始");

	Application app(1280, 720, L"CG2");
	if (!app.Init()) {
		Logger::Write("App 初期化失敗");
		Logger::Shutdown();
		return false;
	}
	app.Run();
	app.Shutdown();

	Logger::Write("アプリ終了");
	Logger::Shutdown();
	return 0;
}