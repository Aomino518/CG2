#include "SeekerEngine.h"
#include <dxgidebug.h>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxcapi.h>
#pragma comment(lib, "Dbghelp.lib")

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻をなめに入れたファイルを作成、Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMilliseconds);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
	minidumpInfomation.ThreadId = threadId;
	minidumpInfomation.ExceptionPointers = exception;
	minidumpInfomation.ClientPointers = TRUE;
	// Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
	// 他に関連づけられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する。
	return EXCEPTION_EXECUTE_HANDLER;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
	CoInitializeEx(0, COINIT_MULTITHREADED);
	SetUnhandledExceptionFilter(ExportDump);
	SeekerEngine engine;
	engine.Init();

	// ImGui初期化
	ImGuiManager imgui;
	imgui.Init(engine.GetApp(), engine.GetGraphics());

	// XAudio2の初期化
	std::unique_ptr<Sound> bgm = std::make_unique<Sound>();
	std::unique_ptr<Sound> se = std::make_unique<Sound>();
	bgm->SetCommon(engine.GetSoundCommon());
	se->SetCommon(engine.GetSoundCommon());
	// 音声読み込み
	SoundData sHAudio1 = bgm->SoundLoad("resources/c21.mp3");
	SoundData sHAudio2 = bgm->SoundLoad("resources/koharubiyori.mp3");
	SoundData sHAudio3 = se->SoundLoad("resources/gold.mp3");
	SoundData sHAudio4 = se->SoundLoad("resources/se_itemget.wav"); 

	// Sprite
	/*Vector2 positoin = {0.0f, 0.0f};
	float rotation = 0.0f;
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
	uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/uvChecker.png");
	sprite->Create(tHChecker, positoin, Color::WHITE);
	sprite->SetRotation(rotation);*/

	// ドラッグドロップで生成するSpriteを格納
	std::vector<std::unique_ptr<Sprite>> sprites;

	// Model
	std::unique_ptr<Entity3D> entity = std::make_unique<Entity3D>();
	ModelManager::GetInstance()->LoadModel("axis.obj");
	entity->Init(engine.GetEntityCommon());
	entity->SetModel("axis.obj");
	entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

	// カメラ
	std::unique_ptr<Camera> camera = std::make_unique<Camera>();
	Vector3 cameraPos = { 0.0f, 0.0f, -10.0f };
	Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };
	camera->SetRotate(cameraRotate);
	camera->SetTranslate(cameraPos);
	engine.GetEntityCommon()->SetDefaultCamera(camera.get());

	imgui.LoadAssets("resources");

	// ウィンドウの×ボタンが押されるまでループ
	while (engine.GetApp()->ProcessMessage()) {
		engine.Update();

		/*-- 更新処理 --*/
		if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
			bgm->SoundPlay(sHAudio1, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_N)) {
			bgm->SoundPlay(sHAudio2, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_M)) {
			se->SoundPlay(sHAudio3, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_V)) {
			se->SoundPlay(sHAudio4, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_B)) {
			bgm->SoundStop();
			se->SoundStop();
		}

		camera->SetRotate(cameraRotate);
		camera->SetTranslate(cameraPos);
		camera->Update();

		//sprite->Update();
		for (auto& sprite : sprites) {
			sprite->Update();
		}

		entity->SetCamera(camera.get());
		entity->Update();

		imgui.BegineFrame();

		imgui.AssetBrowserDraw();
		imgui.SceneViewDraw(sprites);
		imgui.Inspector(sprites);

		/*imgui.BegineInspector();
		imgui.CameraSetting(cameraPos, cameraRotate);
		imgui.SpriteSetting("uvChecker", sprite.get());
		imgui.ModelSetting("axis.obj", entity.get());
		imgui.EndInspector();*/

		imgui.Stats();
		imgui.EndFrame();

		/*-- 描画処理 --*/
		engine.BegineFrame();

		engine.GetEntityCommon()->DrawCommon();
		entity->Draw();

		engine.GetSpriteCommon()->DrawCommon();
		//sprite->Draw();
		for (auto& sprite : sprites) {
			sprite->Draw();
		}

		imgui.Draw();
		engine.EndFrame();
	}
	
	imgui.Shutdown();

	engine.Shutdown();
	return 0;
}