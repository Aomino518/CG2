#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include "Graphics.h"
#include <wrl.h>

class PsoBuilder
{
public:
	void Init(Graphics* graphics);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC CreatePsoDesc(
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
		D3D12_INPUT_LAYOUT_DESC inputLayout,
		Microsoft::WRL::ComPtr<IDxcBlob> vsBlob,
		Microsoft::WRL::ComPtr<IDxcBlob> psBlob, 
		D3D12_BLEND_DESC blendState,
		D3D12_RASTERIZER_DESC rasterizerDesc
		);

	Microsoft::WRL::ComPtr<ID3D12PipelineState> BuildPso(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);

private:
	Graphics* graphics_;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_{};
};

