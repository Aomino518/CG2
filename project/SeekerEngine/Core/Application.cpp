#include "Application.h"
#include "Graphics.h"
#include <format>
#include <sstream>
#include <dxcapi.h>
#include <vector>
#include "MathFunc.h"
#include "DebugCamera.h"
#define _USE_MATH_DEFINES 
#include <math.h>
#include "Logger.h"
#pragma comment(lib, "winmm.lib")

#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

int Application::clientWidth_ = 1280;
int Application::clientHeight_ = 720;

Application* Application::GetInstance() {
	static Application instance;
	return &instance;
}

LRESULT Application::WindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) {
		return true;
	}
#endif

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
	case WM_DESTROY:
		// OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		uint32_t width = LOWORD(lp);
		uint32_t height = HIWORD(lp);

		clientWidth_ = width;
		clientHeight_ = height;

		if (width != 0 && height != 0 && Graphics::GetInstance()->IsInit()) {
			Graphics::GetInstance()->Resize(width, height);
		}
		return 0;
	}

	// 標準のメッセージ処理行う
	return DefWindowProc(hWnd, msg, wp, lp);
}

void Application::Init(int width, int height, const wchar_t* title)
{
#ifdef NDEBUG
	style_ = WS_POPUP;

	HMONITOR monitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);

	clientWidth_ = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	clientHeight_ = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
#else
	clientWidth_ = width;
	clientHeight_ = height;
	style_ = WS_OVERLAPPEDWINDOW;
#endif

	title_ = title;

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

#ifdef NDEBUG
	/*--ウィンドウサイズを決定--*/
	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, clientWidth_, clientHeight_ };

	int windowX = monitorInfo.rcMonitor.left;
	int windowY = monitorInfo.rcMonitor.top;
	int windowWidth = clientWidth_;
	int windowHeight = clientHeight_;
#else
	/*--ウィンドウサイズを決定--*/
	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, clientWidth_, clientHeight_ };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, style_, false);

	int windowX = CW_USEDEFAULT;
	int windowY = CW_USEDEFAULT;
	int windowWidth = wrc.right - wrc.left;
	int windowHeight = wrc.bottom - wrc.top;
#endif
	
	/*--ウィンドウを生成する--*/
	hwnd_ = CreateWindow(
		className_.c_str(), // 利用するクラス名
		title_.c_str(), // タイトルバーの文字
		style_, // よく見るウィンドウスタイル
		windowX, // 表示X座標(Windowsに任せる)
		windowY, // 表示Y座標(WindowsOSに任せる)
		windowWidth, // ウィンドウ横幅
		windowHeight, // ウィンドウ縦幅
		nullptr, // 親ウィンドウハンドル
		nullptr, // メニューハンドル
		hInstance_, // インスタンスハンドル
		this // オプション
	);

	// ゲッター関数で取得するために代入
	hInstance_ = wndclass.hInstance;

	// システムタイマーの分解能を上げる
	timeBeginPeriod(1);

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
	Logger::Write("Application Shutdown");
}
