#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>
#include <thread>
#include "D3DResourceLeakChecker.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

class Graphics
{
public:
	static Graphics* GetInstance();

	// 最大SRV数 (最大テクスチャ枚数)
	static constexpr uint32_t kMaxSRVCount = 4096;

	bool Init(HWND hwnd, uint32_t width, uint32_t height, bool enableDebug = true);
	void Shutdown();

	void BeginFrame();
	void EndFrame();

	// 同期待ち
	void WaitGPU();

	// ゲッター
	static ID3D12Device* GetDevice() { return device_.Get(); }
	static ID3D12GraphicsCommandList* GetCmdList() { return cmdList_.Get(); }
	ID3D12DescriptorHeap* GetSrvHeap() const { return srvHeap_.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentRTV() const { return rtvHandles_[backBufferIndex_]; }
	D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle() const { return dsvHeap_->GetCPUDescriptorHandleForHeapStart(); }

	uint32_t GetSRVDescriptorSize() const { return descSizeSRV_; }
	uint32_t GetRTVDescriptorSize() const { return descSizeRTV_; }
	uint32_t GetDSVDescriptorSize() const { return descSizeDSV_; }

	static uint32_t GetWidth() { return width_; }
	static uint32_t GetHeight() { return height_; }

	D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc() const { return depthStencilDesc; }
	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc; }
	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const { return rtvDesc; }

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVHeap() const { return srvHeap_; }
	uint32_t GetDescriptorSizeSRV() const { return descSizeSRV_; }

	D3D12_VIEWPORT GetViewport() const { return viewport_; }
	D3D12_RECT GetScissorRect() const { return scissorRect_; }

private:
	bool CreateDevice(bool enableDebug);
	bool CreateSwapChain();
	bool CreateHeapsAndTargets();
	bool CreateCommands();
	bool CreateSyncObjects();
	bool CreateViewport();
	bool CreateScissorRect();
	bool CreateImGuiInit();

	// FPSの固定初期化
	void InitFixFPS();
	// FPS固定更新
	void UpdateFixFPS();

	// 基本
	HWND hwnd_;
	static uint32_t width_;
	static uint32_t height_;
	Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_;
	static Microsoft::WRL::ComPtr<ID3D12Device> device_;

	// コマンド
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator_;
	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	// スワップチェーン
	static constexpr uint32_t kBufferCount = 2;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers_[kBufferCount];
	uint32_t backBufferIndex_ = 0;

	// ヒープ・ビュー
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[kBufferCount]{};
	Microsoft::WRL::ComPtr<ID3D12Resource> depthTex_;

	uint32_t descSizeRTV_ = 0;
	uint32_t descSizeDSV_ = 0;
	uint32_t descSizeSRV_ = 0;

	// 同期
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_RESOURCE_BARRIER barrier{};

	// ビューポート
	D3D12_VIEWPORT viewport_{};

	// シザー矩形
	D3D12_RECT scissorRect_{};

	// 画面クリアカラー
	const float clear[4] = { 0.1f, 0.25f, 0.5f, 1.0f };

	// 記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;
};

