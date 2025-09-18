#include "App.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

App::App(int width, int height, const wchar_t* title)
	: hInstance_(GetModuleHandle(nullptr)), title_(title ? title : L"CG2"),
		clientWidth_(width), clientHeight_(height) {}

App::~App()
{
	Shutdown();
}

bool App::Init()
{
	/*--ウィンドウクラスの登録--*/
	WNDCLASS wc{};

	// ウィンドウプロシージャ
	wc.lpfnWndProc = App::WndProcSetup;

	// ウィンドウクラス名
	wc.lpszClassName = className_.c_str();

	// インスタンスハンドル
	wc.hInstance = hInstance_;

	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);

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

bool App::ProcessMessage()
{
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) { 
			isRunning_ = false; return false; 
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return isRunning_;
}

void App::Shutdown()
{
	if (hwnd_) {
		DestroyWindow(hwnd_);
		hwnd_ = nullptr;
	}
	UnregisterClass(className_.c_str(), hInstance_);
	isRunning_ = false;
}

LRESULT App::WndProcSetup(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == WM_NCCREATE) {
		auto cs = reinterpret_cast<CREATESTRUCT*>(lp);
		auto that = reinterpret_cast<App*>(cs->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&App::WndProcThunk));
		return that->WndProc(hWnd, msg, wp, lp);
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT App::WndProcThunk(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	auto that = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return that ? that->WndProc(hWnd, msg, wp, lp) : DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) {
		return true;
	}

	switch (msg) {
	case WM_SIZE:
		clientWidth_ = LOWORD(lp);
		clientHeight_ = HIWORD(lp);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}


