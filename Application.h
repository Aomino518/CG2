#pragma once
#include <Windows.h>
#include "Graphics.h"
#include <string>
#include <cassert>
#include "Inputs.h"
#include "Sound.h"
#include "DxcCompiler.h"
#include "RootSignatureFactory.h"
#include "InputLayout.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

class Application
{
public:
	Application(int width, int height, const wchar_t* title);
	~Application();

	bool Init();			// ウィンドウ作成
	bool ProcessMessage();  // メッセージポンプ
	void Run();
	void Shutdown();		// 明示終了

	// アクセッサ
	HINSTANCE GetHInstance() const { return hInstance_; }
	HWND GetHWND() const { return hwnd_; }
	int GetWidth() const { return clientWidth_; }
	int GetHeight() const { return clientHeight_; }

private:
	HINSTANCE hInstance_ = nullptr;
	HWND hwnd_ = nullptr;
	std::wstring className_ = L"CG2WindowClass";
	std::wstring title_ = L"CG2";
	int clientWidth_ = 1280;
	int clientHeight_ = 720;
	DWORD style_ = WS_OVERLAPPEDWINDOW;
	bool isRunning_ = false;

	static LRESULT CALLBACK WndProcSetup(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT CALLBACK WndProcThunk(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	Graphics graphics_;
	Inputs input_;
	Sound xAudio2_;
	DxcCompiler dxcCompiler_;
	RootSignatureFactory rootSignatureFactory_;
	InputLayout inputLayout_;
};

