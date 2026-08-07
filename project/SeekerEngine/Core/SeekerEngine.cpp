#include "SeekerEngine.h"

void SeekerEngine::Init()
{
	StartupManager::Init();
	Logger::Init();
	Logger::Write("アプリ開始");

	Application::GetInstance()->Init(1280, 720, L"流撃");

	// graphicsの初期化
	Graphics::GetInstance()->Init(true);

	SrvManager::GetInstance()->Init();

	// DxcCompilerの初期化
	dxcCompiler_.Init();

	//DirectInput初期化
	Input::GetInstance()->Init();
	TextureManager::GetInstance()->Init();
	ModelManager::GetInstance()->Init();
  
	// RootSignature作成
	rootSignatureFactory_.Init();
	rs3D_ = rootSignatureFactory_.Create3D();
	rs2D_ = rootSignatureFactory_.Create2D();
	rsParticle_ = rootSignatureFactory_.CreateParticle3D();

	SoundManager::GetInstance()->Init();

	// スプライト共通部の作成
	SpriteCommon::GetInstance()->Init(dxcCompiler_, rs2D_.Get());

	// モデル共通部の作成
	Entity3DCommon::GetInstance()->Init(dxcCompiler_, rs3D_.Get());

	ParticleManager::GetInstance()->Init(dxcCompiler_, rsParticle_.Get());
	LightManager::GetInstance()->Init();
	ImGuiManager::GetInstance()->Init();
}

void SeekerEngine::Update()
{
	Input::GetInstance()->Update();
}

void SeekerEngine::Shutdown()
{
	SceneManager::GetInstance()->Shutdown();
	ImGuiManager::GetInstance()->Shutdown();
	LightManager::GetInstance()->Shutdown();
	ParticleManager::GetInstance()->Shutdown();
	Entity3DCommon::GetInstance()->Shutdown();
	SpriteCommon::GetInstance()->Shutdown();
	SoundManager::GetInstance()->Shutdown();
	ModelManager::GetInstance()->Shutdown();
	TextureManager::GetInstance()->Shutdown();
	Input::GetInstance()->Shutdown();
	SrvManager::GetInstance()->Shutdown();
  
	Graphics::GetInstance()->Shutdown();
	Application::GetInstance()->Shutdown();

	Logger::Write("アプリ終了");
	Logger::Shutdown();
	StartupManager::Shutdown();
}

void SeekerEngine::BegineFrame()
{
	Graphics::GetInstance()->BeginFrame();
}

void SeekerEngine::EndFrame()
{
	Graphics::GetInstance()->EndFrame();
}