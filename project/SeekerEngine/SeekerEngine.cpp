#include "SeekerEngine.h"

void SeekerEngine::Init()
{
	StartupManager::Init();
	Logger::Init();
	Logger::Write("アプリ開始");

	app_ = std::make_unique<Application>(1280, 720, L"CG2");
	app_->Init();

	// graphicsの初期化
	graphics_ = std::make_unique<Graphics>();
	graphics_->Init(app_->GetHWND(), app_->GetWidth(), app_->GetHeight(), true);

	SrvManager::GetInstance()->Init(graphics_.get());

	// DxcCompilerの初期化
	dxcCompiler_.Init();

	//DirectInput初期化
	Input::GetInstance()->Init(app_.get());
	TextureManager::GetInstance()->Init(graphics_.get());
	ModelManager::GetInstance()->Init(graphics_.get());
  
	// RootSignature作成
	rootSignatureFactory_.Init(graphics_.get());
	rs3D_ = rootSignatureFactory_.Create3D();
	rs2D_ = rootSignatureFactory_.Create2D();
	rsParticle_ = rootSignatureFactory_.CreateParticle3D();

	// SoundCommon作成
	SoundCommon::GetInstance()->Init();
	
	// スプライト共通部の作成
	SpriteCommon::GetInstance()->Init(graphics_.get(), dxcCompiler_, rs2D_.Get());

	// モデル共通部の作成
	Entity3DCommon::GetInstance()->Init(graphics_.get(), dxcCompiler_, rs3D_.Get());

	ParticleManager::GetInstance()->Init(graphics_.get(), dxcCompiler_, rsParticle_.Get());
	LightManager::GetInstance()->Init();
	ImGuiManager::GetInstance()->Init(app_.get(), graphics_.get());
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
	ModelManager::GetInstance()->Shutdown();
	TextureManager::GetInstance()->Shutdown();

	Input::GetInstance()->Shutdown();

	SoundCommon::GetInstance()->Shutdown();
	
	SrvManager::GetInstance()->Shutdown();
  
	graphics_->Shutdown();

	Logger::Write("AppのShutdown");
	app_->Shutdown();

	Logger::Write("アプリ終了");
	Logger::Shutdown();
	StartupManager::Shutdown();
}

void SeekerEngine::BegineFrame()
{
	graphics_->BeginFrame();
}

void SeekerEngine::EndFrame()
{
	graphics_->EndFrame();
}