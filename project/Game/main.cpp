#include "SEFramework.h"
#include "Game.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	std::unique_ptr<SEFramework> seFw = std::make_unique<Game>();
	seFw->Run();
}