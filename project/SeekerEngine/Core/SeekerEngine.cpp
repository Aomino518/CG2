#include "SeekerEngine.h"
#include "Application.h"
#include "Logger.h"
#include "MathFunc.h"
#include "DebugCamera.h"
#include "StringUtil.h"
#include "Input.h"
#include "Sound.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Entity3DCommon.h"
#include "Entity3D.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include "StartupManager.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "LightManager.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "CameraManager.h"
#include "EmitterManager.h"
#include "WorldFieldManager.h"
#include "DebugDraw.h"
#include "DebugDraw2D.h"
#include "DebugDraw3D.h"
#include "Particle2DManager.h"

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
	rsParticle2D_ = rootSignatureFactory_.CreateParticle2D();
	rsDebugShape2D_ = rootSignatureFactory_.CreateDebugShape2D();
	rsDebugShape3D_ = rootSignatureFactory_.CreateDebugShape3D();

	SoundManager::GetInstance()->Init();

	CameraManager::GetInstance()->Init();

	DebugDraw2D::GetInstance()->Init(dxcCompiler_, rsDebugShape2D_.Get());
	DebugDraw3D::GetInstance()->Init(dxcCompiler_, rsDebugShape3D_.Get());
	// スプライト共通部の作成
	SpriteCommon::GetInstance()->Init(dxcCompiler_, rs2D_.Get());

	// モデル共通部の作成
	Entity3DCommon::GetInstance()->Init(dxcCompiler_, rs3D_.Get());

	ParticleManager::GetInstance()->Init(dxcCompiler_, rsParticle_.Get());
	Particle2DManager::GetInstance()->Init(dxcCompiler_, rsParticle2D_.Get());
	LightManager::GetInstance()->Init();
	ImGuiManager::GetInstance()->Init();
}

void SeekerEngine::Update()
{
	Input::GetInstance()->Update();
	DebugDraw::Update();
	LightManager::GetInstance()->Update();
	CameraManager::GetInstance()->Update();
}

void SeekerEngine::Shutdown()
{
	SceneManager::GetInstance()->Shutdown();
	ImGuiManager::GetInstance()->Shutdown();
	LightManager::GetInstance()->Shutdown();
	Particle2DManager::GetInstance()->Shutdown();
	ParticleManager::GetInstance()->Shutdown();
	Entity3DCommon::GetInstance()->Shutdown();
	SpriteCommon::GetInstance()->Shutdown();
	DebugDraw3D::GetInstance()->Shutdown();
	DebugDraw2D::GetInstance()->Shutdown();
	CameraManager::GetInstance()->Shutdown();
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

Application* SeekerEngine::GetApp() const
{
	return Application::GetInstance();
}

Graphics* SeekerEngine::GetGraphics() const
{
	return Graphics::GetInstance();
}

DxcCompiler SeekerEngine::GetDxcCompiler() const {
	return dxcCompiler_;
}

RootSignatureFactory SeekerEngine::GetRootSig() const
{
	return rootSignatureFactory_;
}
