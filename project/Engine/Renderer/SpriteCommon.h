#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <Graphics.h>
#include <DxcCompiler.h>
#include <InputLayout.h>
#include <PsoBuilder.h>

class SpriteCommon {
public:
	void Init(Graphics* graphics, DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);
	void DrawCommon();
	ID3D12PipelineState* GetPipelineState() { return pipelineState_.Get(); }

	Graphics* GetGraphics() const { return graphics_; }

private:
	// グラフィックパイプラインの作成
	void CreateGraphicPipeline(Graphics* graphics, DxcCompiler dxcCompiler);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Graphics* graphics_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso2D_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};
