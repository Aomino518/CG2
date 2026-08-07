#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DxcCompiler.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "BlendStateUtils.h"
#include <unordered_map>

class Graphics;
class SpriteCommon {
public:
	// シングルトンインスタンスの取得
	static SpriteCommon* GetInstance();
	void Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);
	void ApplyPipeline();

	void Shutdown();

	ID3D12PipelineState* GetPipelineState() { return pipelineState_.Get(); }

	BlendMode& GetBlendMode() { return mode_; }

	void SetBlendMode(BlendMode mode);

private:
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;

	// グラフィックパイプラインの作成
	void CreateGraphicPipeline(DxcCompiler dxcCompiler);

	void RebuildPso();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Graphics* graphics_;

	Microsoft::WRL::ComPtr<IDxcBlob> vs2DBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> ps2DBlob_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso2D_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	D3D12_BLEND_DESC blendDesc_{};
	BlendMode mode_ = kBlendModeNormal;
	std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
};
