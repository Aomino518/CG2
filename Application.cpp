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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma endregion

Application::Application(int width, int height, const wchar_t* title)
	: hInstance_(GetModuleHandle(nullptr)), title_(title ? title : L"CG2"),
		clientWidth_(width), clientHeight_(height) {}


Application::~Application()
{
	Shutdown();
}

bool Application::Init()
{
	/*--ウィンドウクラスの登録--*/

	// ウィンドウプロシージャ
	wndclass.lpfnWndProc = Application::WndProcSetup;

	// ウィンドウクラス名
	wndclass.lpszClassName = className_.c_str();

	// インスタンスハンドル
	wndclass.hInstance = hInstance_;

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

	if (!hwnd_) {
		return false;
	}

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);
	isRunning_ = true;
	return true;
}

bool Application::ProcessMessage()
{
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) { 
			isRunning_ = false; 
			return false; 
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return isRunning_;
}

void Application::Shutdown()
{
	CloseWindow(hwnd_);
	hwnd_ = nullptr;
	UnregisterClass(className_.c_str(), hInstance_);

	// COMの終了処理
	CoUninitialize();

	isRunning_ = false;
}

LRESULT Application::WndProcSetup(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == WM_NCCREATE) {
		auto cs = reinterpret_cast<CREATESTRUCT*>(lp);
		auto that = reinterpret_cast<Application*>(cs->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Application::WndProcThunk));
		return that->WndProc(hWnd, msg, wp, lp);
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT Application::WndProcThunk(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	auto that = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return that ? that->WndProc(hWnd, msg, wp, lp) : DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT Application::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) {
		return true;
	}

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
	case WM_CLOSE:
		// ここで明示的に破棄
		DestroyWindow(hWnd);
		return 0;
	case WM_SIZE:
		clientWidth_ = LOWORD(lp);
		clientHeight_ = HIWORD(lp);
		return 0;
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理行う
	return DefWindowProc(hWnd, msg, wp, lp);
}