#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <DxcCompiler.h>
#include <InputLayout.h>
#include "PsoBuilder.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "CameraManager.h"
#include "BlendStateUtils.h"

class Graphics;
class Entity3DCommon
{
public:
	// シングルトンインスタンスの取得
	static Entity3DCommon* GetInstance();

	void Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);

	void ApplyPipeline();

	void Shutdown();

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCmdList() const { return cmdList_; }

	// Getter
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	DebugCamera* GetDebugCamera() const { return debugCamera_; }
	CameraManager* GetCameraManager() const { return cameraManager_; }
	BlendMode& GetBlendMode() { return mode_; }
	// Setter
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }
	void SetCameraManager(CameraManager* cameraManager) { this->cameraManager_ = cameraManager; }
	void SetDebugCamera(DebugCamera* debugCamera) { this->debugCamera_ = debugCamera; }
	void SetBlendMode(BlendMode mode);

private:
	Entity3DCommon() = default;
	~Entity3DCommon() = default;
	Entity3DCommon(const Entity3DCommon&) = delete;
	Entity3DCommon& operator=(const Entity3DCommon&) = delete;

	// グラフィックパイプラインの作成
	void CreateGraphicPipeline(DxcCompiler dxcCompiler);

	void RebuildPso();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Graphics* graphics_;

	Microsoft::WRL::ComPtr<IDxcBlob> vs3DBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> ps3DBlob_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso3D_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	Camera* defaultCamera_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	CameraManager* cameraManager_ = nullptr;

	D3D12_BLEND_DESC blendDesc_{};
	BlendMode mode_ = kBlendModeNormal;
	std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
};

