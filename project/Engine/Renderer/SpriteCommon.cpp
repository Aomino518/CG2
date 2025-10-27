#include "SpriteCommon.h"


void SpriteCommon::Init(Graphics* graphics, DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature)
{
	graphics_ = graphics;
	rootSignature_ = rootSignature;
	CreateGraphicPipeline(graphics, dxcCompiler);
	cmdList_ = graphics->GetCmdList();
}

void SpriteCommon::DrawCommon()
{
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetPipelineState(pso2D_.Get());
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SpriteCommon::CreateGraphicPipeline(Graphics* graphics, DxcCompiler dxcCompiler)
{
	InputLayout inputLayout;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc2D{};
	inputLayoutDesc2D = inputLayout.CreateInputLayout2D();

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vs2DBlob = dxcCompiler.CompileShader(L"resources/hlsl/Object2D.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> ps2DBlob = dxcCompiler.CompileShader(L"resources/hlsl/Object2D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	PsoBuilder builder;
	builder.Init(graphics);
	// 2D用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2D{};
	psoDesc2D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDesc2D,
		vs2DBlob,
		ps2DBlob,
		blendDesc,
		rasterizerDesc
	);

	pso2D_ = builder.BuildPso(psoDesc2D);
	Logger::Write("PSO2D生成完了");
}
