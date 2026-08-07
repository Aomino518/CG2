#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxcompiler.lib")

class Graphics;
class RootSignatureFactory
{
public:
	void Init();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> Create2D();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> Create3D();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateParticle3D();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateParticle2D();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateDebugShape2D();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateDebugShape3D();

private:

	Graphics* graphics_;
};

