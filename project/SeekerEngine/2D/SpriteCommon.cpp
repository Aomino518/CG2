#include "SpriteCommon.h"
#include "Graphics.h"
#include "Logger.h"

SpriteCommon* SpriteCommon::GetInstance()
{
	static SpriteCommon instance;
	return &instance;
}

void SpriteCommon::Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature)
{
	graphics_ = Graphics::GetInstance();
	rootSignature_ = rootSignature;
	CreateGraphicPipeline(dxcCompiler);
	cmdList_ = Graphics::GetCmdList();
}

void SpriteCommon::ApplyPipeline()
{
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetPipelineState(pso2D_.Get());
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SpriteCommon::Shutdown()
{
	pso2D_.Reset();
	vs2DBlob_.Reset();
	ps2DBlob_.Reset();
	rootSignature_.Reset();
	graphics_ = nullptr;
	psoCache_.clear();
	pipelineState_.Reset();
	cmdList_.Reset();
	Logger::Write("SpriteCommon Shutdown");
}	

void SpriteCommon::RebuildPso()
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = false;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	InputLayout inputLayout;
	D3D12_INPUT_LAYOUT_DESC layout = inputLayout.CreateInputLayout2D();

	blendDesc_ = CreateBlendDesc(mode_);

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	builder.Init(graphics_);
	psoDesc = builder.CreatePsoDesc(
		rootSignature_.Get(),
		layout,
		vs2DBlob_,
		ps2DBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	pso2D_ = builder.BuildPso(psoDesc);
}

void SpriteCommon::SetBlendMode(BlendMode mode)
{
	if (mode_ == mode)
	{
		return;
	}

	mode_ = mode;

	if (psoCache_.contains(mode)) {
		pso2D_ = psoCache_[mode];
	} else {
		RebuildPso();
		psoCache_[mode] = pso2D_;
	}
}

void SpriteCommon::CreateGraphicPipeline(DxcCompiler dxcCompiler)
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = false;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc2D{};
	InputLayout inputLayout;
	inputLayoutDesc2D = inputLayout.CreateInputLayout2D();

	// BlendStateの設定
	// すべての色要素を書き込む
	blendDesc_ = CreateBlendDesc(mode_);
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	vs2DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Object2D.VS.hlsl", L"vs_6_0");
	ps2DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Object2D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	PsoBuilder builder;
	builder.Init(graphics_);
	// 2D用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2D{};
	psoDesc2D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDesc2D,
		vs2DBlob_,
		ps2DBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	pso2D_ = builder.BuildPso(psoDesc2D);
	Logger::Write("PSO2D生成完了");
}
