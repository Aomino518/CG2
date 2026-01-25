#pragma once
#include "Application.h"
#include "Logger.h"
#include "Matrix.h"
#include "DebugCamera.h"
#include "StringUtil.h"
#include "Input.h"
#include "SoundCommon.h"
#include "Sound.h"
#include "DxcCompiler.h"
#include "RootSignatureFactory.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
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

class SeekerEngine
{
public:
	void Init();
	void Update();
	void Shutdown();

	void BegineFrame();
	void EndFrame();

	// Getter
	Application* GetApp() const { return app_.get(); }
	Graphics* GetGraphics() const { return graphics_.get(); }
	DxcCompiler GetDxcCompiler() const { return dxcCompiler_; }
	RootSignatureFactory GetRootSig() const { return rootSignatureFactory_; }

private:
	std::unique_ptr<Application> app_;
	std::unique_ptr<Graphics> graphics_;
	
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs3D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs2D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rsParticle_;

	DxcCompiler dxcCompiler_;
	RootSignatureFactory rootSignatureFactory_;
	InputLayout inputLayout_;
	PsoBuilder psoBuilder_;

};