#pragma once
#include <d3d12.h>
#include <wrl.h>
#pragma comment(lib, "d3d12.lib")

class InputLayout
{
public:
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout3D();
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout2D();

private:
	D3D12_INPUT_ELEMENT_DESC inputElementDescs3D_[3];
	D3D12_INPUT_ELEMENT_DESC inputElementDescs2D_[2];
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc3D_{};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc2D_{};
};

