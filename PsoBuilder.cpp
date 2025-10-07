#include "PsoBuilder.h"
#include <cassert>

void PsoBuilder::Init(Graphics* graphics)
{
	graphics_ = graphics;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoBuilder::CreatePsoDesc(
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature, 
	D3D12_INPUT_LAYOUT_DESC inputLayout, 
	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, 
	Microsoft::WRL::ComPtr<IDxcBlob> psBlob, 
	D3D12_BLEND_DESC blendState, 
	D3D12_RASTERIZER_DESC rasterizerDesc
	)
{
	desc_.pRootSignature = rootSignature.Get();
	desc_.InputLayout = inputLayout;
	desc_.VS = { vsBlob->GetBufferPointer(),
	vsBlob->GetBufferSize() };
	desc_.PS = { psBlob->GetBufferPointer(),
	psBlob->GetBufferSize() };
	desc_.BlendState = blendState;
	desc_.RasterizerState = rasterizerDesc;

	// DepthStencilの設定
	desc_.DepthStencilState = graphics_->GetDepthStencilDesc();
	desc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 書き込むRTVの情報
	desc_.NumRenderTargets = 1;
	desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	desc_.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	desc_.SampleDesc.Count = 1;
	desc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    return desc_;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PsoBuilder::BuildPso(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc)
{
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_ = nullptr;
	HRESULT hr = graphics_->GetDevice()->CreateGraphicsPipelineState(&desc,
		IID_PPV_ARGS(&pso_));
	assert(SUCCEEDED(hr));

	return pso_;
}
