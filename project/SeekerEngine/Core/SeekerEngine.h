#pragma once
#include <wrl.h>
#include "DxcCompiler.h"
#include "RootSignatureFactory.h"
#include "InputLayout.h"
#include "PsoBuilder.h"

class Application;
class Graphics;
class SeekerEngine
{
public:
	void Init();
	void Update();
	void Shutdown();

	void BegineFrame();
	void EndFrame();

	// Getter
	Application* GetApp() const;
	Graphics* GetGraphics() const;
	DxcCompiler GetDxcCompiler() const;
	RootSignatureFactory GetRootSig() const;

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs3D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs2D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rsParticle_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rsParticle2D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rsDebugShape2D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rsDebugShape3D_;

	DxcCompiler dxcCompiler_;
	RootSignatureFactory rootSignatureFactory_;
	InputLayout inputLayout_;
	PsoBuilder psoBuilder_;

};