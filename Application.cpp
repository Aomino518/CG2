#include "Application.h"
#include <format>
#include <dxgidebug.h>
#include <dbghelp.h>
#include <strsafe.h>
#include <sstream>
#include <dxcapi.h>
#include <vector>
#include "Matrix.h"
#include "DebugCamera.h"
#define _USE_MATH_DEFINES 
#include <math.h>
#include "Logger.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Application::WindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) {
		Logger::Write("ループ開始");
		return true;
	}

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
	case WM_DESTROY:
		// OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理行う
	return DefWindowProc(hWnd, msg, wp, lp);
}

Application::Application(int width, int height, const wchar_t* title)
{
	clientWidth_ = width;
	clientHeight_ = height;
	title_ = title;
}

Application::~Application()
{
}

void Application::Init()
{
	/*--ウィンドウクラスの登録--*/

	// ウィンドウプロシージャ
	wndclass.lpfnWndProc = WindowProc;

	// ウィンドウクラス名
	wndclass.lpszClassName = className_.c_str();

	// インスタンスハンドル
	wndclass.hInstance = GetModuleHandle(nullptr);;

	// カーソル
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wndclass);


	/*--ウィンドウサイズを決定--*/
	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, clientWidth_, clientHeight_ };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, style_, false);

	/*--ウィンドウを生成する--*/
	hwnd_ = CreateWindow(
		className_.c_str(), // 利用するクラス名
		title_.c_str(), // タイトルバーの文字
		style_, // よく見るウィンドウスタイル
		CW_USEDEFAULT, // 表示X座標(Windowsに任せる)
		CW_USEDEFAULT, // 表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left, // ウィンドウ横幅
		wrc.bottom - wrc.top, // ウィンドウ縦幅
		nullptr, // 親ウィンドウハンドル
		nullptr, // メニューハンドル
		hInstance_, // インスタンスハンドル
		this // オプション
	);

	// ゲッター関数で取得するために代入
	hInstance_ = wndclass.hInstance;

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);
}

bool Application::ProcessMessage()
{
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) { 
			return false; 
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

void Application::Shutdown()
{
	CloseWindow(hwnd_);
	hwnd_ = nullptr;
	UnregisterClass(className_.c_str(), hInstance_);

	// COMの終了処理
	CoUninitialize();
}
