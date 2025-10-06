#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include "Graphics.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxcompiler.lib")

class RootSignatureFactory
{
public:
	void Init(Graphics* graphics);

	//Microsoft::WRL::ComPtr<ID3D12RootSignature>
		//CreateFor2D(UINT srvCount = 1, bool denyGS = true) const;

	//Microsoft::WRL::ComPtr<ID3D12RootSignature>
		//CreateFor3D(UINT srvCount = 128, bool denyGS = true) const;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>
		CreateCommon();

private:

	Graphics* graphics_;
};

