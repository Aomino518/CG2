#include "Graphics.h"
#include "Application.h"
#include "Logger.h"
#include "StringUtil.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include <format>
#include <cassert>
#include <thread>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Device> Graphics::device_ = nullptr;
ComPtr<ID3D12GraphicsCommandList> Graphics::cmdList_ = nullptr;
uint32_t Graphics::width_ = 0;
uint32_t Graphics::height_ = 0;

Graphics* Graphics::GetInstance() {
	static Graphics instance;
	return &instance;
}

bool Graphics::Init(bool enableDebug)
{
	// FPSの固定初期化
	InitFixFPS();

	width_ = Application::GetInstance()->GetWidth();
	height_ = Application::GetInstance()->GetHeight();
	hwnd_ = Application::GetInstance()->GetHWND();

#ifdef _DEBUG
	if (enableDebug) {
		ComPtr<ID3D12Debug1> debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			// デバッグレイヤー有効化する
			debugController->EnableDebugLayer();
			debugController->SetEnableGPUBasedValidation(true);
		}
	}
#endif

	// グラフィックスに関連するオブジェクトを生成・管理するインターフェイス(DXGIFactory)を生成
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&factory_));
	assert(SUCCEEDED(hr));

	if (!CreateDevice(enableDebug)) {
		Logger::Write("Generation failed Device");
		return false;
	}
	Logger::Write("Complete Create Device");

	if (!CreateCommands()) {
		Logger::Write("Generation failed Commands");
		return false;
	}
	Logger::Write("Complete Create Commands");

	if (!CreateSwapChain()) {
		Logger::Write("Generation failed SwapChain");
		return false;
	}
	Logger::Write("Complete Create SwapChain");

	if (!CreateHeapsAndTargets()) {
		Logger::Write("Generation failed HeapAndTargetView");
		return false;
	}
	Logger::Write("Complete Create HeapAndTargetView");

	if (!CreateSyncObjects()) {
		Logger::Write("Generation failed Fence");
		return false;
	}
	Logger::Write("Complete Create Fence");

	if (!CreateViewport()) {
		Logger::Write("Generation failed Viewport");
		return false;
	}

	if (!CreateScissorRect()) {
		Logger::Write("Generation failed ScissorRect");
		return false;
	}

	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	// 初期化完了ログ
	Logger::Write("Complete Create D3D12Device!!!");

	return true;
}

void Graphics::Shutdown()
{
	WaitGPU();

	for (auto& bb : backBuffers_) {
		bb.Reset();
	}
	depthTex_.Reset();
	dsvHeap_.Reset();
	rtvHeap_.Reset();

	cmdList_.Reset();
	cmdAllocator_.Reset();
	cmdQueue_.Reset();

	fence_.Reset();
	swapChain_.Reset();
	device_.Reset();
	adapter_.Reset();
	factory_.Reset();

	CloseHandle(fenceEvent_);
	Logger::Write("Graphics Shutdown");
}

void Graphics::BeginFrame()
{
	ResetDrawCallCount();
	// これから書き込むバックバッファのインデックス取得
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
	// TransitionBarrierの設定
	// 今回バリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = backBuffers_[backBufferIndex_].Get();
	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る
	cmdList_->ResourceBarrier(1, &barrier);

	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
	cmdList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex_], false, &dsvHandle);
	cmdList_->ClearRenderTargetView(rtvHandles_[backBufferIndex_], clear, 0, nullptr);
	// 指定した深度で画面をクリアする
	cmdList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画用のDescriptorHeapの設定
	SrvManager::GetInstance()->PreDraw();

	cmdList_->RSSetViewports(1, &viewport_); // Viewportを設定
	cmdList_->RSSetScissorRects(1, &scissorRect_); // Scissorを設定
}

void Graphics::EndFrame()
{
	// TransitionBarrierの設定
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// TransitionBarrierを張る
	cmdList_->ResourceBarrier(1, &barrier);

	//コマンドリストの内容を確定させる。全てコマンドを積んでからCloseすること
	HRESULT hr = cmdList_->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* cmdLists[] = { cmdList_.Get() };
	cmdQueue_->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// FPS固定
	UpdateFixFPS();

	// GPUとOSに画面の交換を行うよう通知する
	swapChain_->Present(1, 0);

	// Fenceの値を更新
	fenceValue_++;
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	cmdQueue_->Signal(fence_.Get(), fenceValue_);

	// Fenceの値が指定したSignal値にたどり着いているか確認する
	// GetCompletedValveの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを指定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		// イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	// 次のフレーム用のコマンドリストを準備
	hr = cmdAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = cmdList_->Reset(cmdAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void Graphics::WaitGPU()
{
	// Fenceの値を更新
	fenceValue_++;
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	cmdQueue_->Signal(fence_.Get(), fenceValue_);

	// Fenceの値が指定したSignal値にたどり着いているか確認する
	// GetCompletedValveの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを指定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		// イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	TextureManager::GetInstance()->ClearIntermediate();
}

static ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height) {
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // Textureの幅
	resourceDesc.Height = height; // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥域 or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能フォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る
	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1,0f (最大値) でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr;
	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void Graphics::Resize(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0) {
		Logger::Write(Logger::LogLevel::Warning, "width,heightが0です");
		return;
	}

	if (!cmdQueue_ || !fence_ || !swapChain_) {
		width_ = width;
		height_ = height;
		return;
	}

	WaitGPU();

	width_ = width;
	height_ = height;

	for (auto& backBuffer : backBuffers_) {
		backBuffer.Reset();
	}

	depthTex_.Reset();

	HRESULT hr = swapChain_->ResizeBuffers(
		kBufferCount,
		width_,
		height_,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0
	);
	assert(SUCCEEDED(hr));

	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	// RTV
	for (uint32_t i = 0; i < kBufferCount; ++i) {
		// SwapChainからResourceを引っ張ってくる
		hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffers_[i]));
		assert(SUCCEEDED(hr));
		rtvHandles_[i] = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
		rtvHandles_[i].ptr += SIZE_T(i) * descSizeRTV_;
		// RTVの設定
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		device_->CreateRenderTargetView(backBuffers_[i].Get(), &rtvDesc, rtvHandles_[i]);
	}

	// DepthStencilTextureをウィンドウのサイズで再作成
	depthTex_ = CreateDepthStencilTextureResource(device_, width_, height_);

	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = GetCPUDescriptorHandle(dsvHeap_, descSizeDSV_, 0);
	// DSVHeapの先頭にDSVを作る
	device_->CreateDepthStencilView(depthTex_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());

	CreateViewport();
	CreateScissorRect();
}

bool Graphics::IsInit() const
{
	return cmdQueue_ && fence_ && swapChain_;
}

ComPtr<ID3D12DescriptorHeap> Graphics::CreateDescriptorHeap(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	/*--ディスクリプタヒープの生成--*/
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr;
	hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}


static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

bool Graphics::CreateDevice(bool enableDebug)
{
#ifdef _DEBUG
	if (enableDebug) {
		Microsoft::WRL::ComPtr<ID3D12Debug1> dbg;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbg)))) {
			dbg->EnableDebugLayer();
			dbg->SetEnableGPUBasedValidation(TRUE);
		}
	}
#endif

	// アダプタ選定
	adapter_ = nullptr;

	for (UINT i = 0; factory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter_)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = adapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの内容をログに出力
			Logger::Write(ConvertString(std::format(L"Use Adapter: {}\n", adapterDesc.Description)));
			break;
		}
		// ソフトウェアアダプタの場合は見なかったことにする
		adapter_ = nullptr;
	}
	// 適切なアダプタが見つからなかったので起動できない
	assert(adapter_ != nullptr);

	if (!adapter_) {
		Logger::Write("ERROR: No suitable hardware adapter found.");
		return false;
	}

	// デバイス作成
	static const D3D_FEATURE_LEVEL kLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};
	static const char* kLevelStr[] = { "12.2", "12.1", "12.0", "11.1", "11.0" };
	const size_t kLevelCount = sizeof(kLevels) / sizeof(kLevels[0]);

	device_.Reset();
	bool created = false;

	for (size_t i = 0; i < kLevelCount; ++i) {
		Microsoft::WRL::ComPtr<ID3D12Device> dev;
		HRESULT hr = D3D12CreateDevice(adapter_.Get(), kLevels[i], IID_PPV_ARGS(&dev));
		if (SUCCEEDED(hr)) {
			device_ = dev;
			Logger::Write(std::format("D3D12CreateDevice OK (FeatureLevel: {})", kLevelStr[i]));
			created = true;
			break;
		} else {
			Logger::Write(std::format("D3D12CreateDevice NG (FeatureLevel: {}), hr=0x{:08X}",
				kLevelStr[i], (unsigned)hr));
		}
	}

	if (!created || !device_) {
		Logger::Write("ERROR: D3D12CreateDevice failed on selected adapter.");
		return false;
	}

#ifdef _DEBUG
	// InfoQueue致命的エラーのみブレーク）
	if (enableDebug) {
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
		}
	}
#endif

	// Descriptorサイズ取得
	descSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	return true;
}

bool Graphics::CreateSwapChain()
{
	/*--スワップチェーンを生成する--*/
	swapChain_ = nullptr;
	swapChainDesc.Width = width_;
	swapChainDesc.Height = height_;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = kBufferCount;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	HRESULT hr = factory_->CreateSwapChainForHwnd(cmdQueue_.Get(), hwnd_, &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));

	assert(SUCCEEDED(hr));
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	return true;
}

bool Graphics::CreateHeapsAndTargets()
{
	/*--ディスクリプタヒープの生成--*/
	rtvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kBufferCount, false);

	// RTV
	for (uint32_t i = 0; i < kBufferCount; ++i) {
		// SwapChainからResourceを引っ張ってくる
		HRESULT hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffers_[i])); 
		assert(SUCCEEDED(hr));
		rtvHandles_[i] = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
		rtvHandles_[i].ptr += SIZE_T(i) * descSizeRTV_;
		// RTVの設定
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		device_->CreateRenderTargetView(backBuffers_[i].Get(), &rtvDesc, rtvHandles_[i]);
	}

	// DepthStencilTextureをウィンドウのサイズで作成
	depthTex_ = CreateDepthStencilTextureResource(device_, width_, height_);

	// DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = GetCPUDescriptorHandle(dsvHeap_, descSizeDSV_, 0);
	// DSVHeapの先頭にDSVを作る
	device_->CreateDepthStencilView(depthTex_.Get(), &dsvDesc, dsvCpuHandle);

	return true;
}

bool Graphics::CreateCommands()
{
	/*--コマンドキューを作成する--*/
	cmdQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&cmdQueue_));

	// コマンドキューの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));

	/*--コマンドアロケータを生成する--*/
	cmdAllocator_ = nullptr;
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator_));

	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	/*--コマンドリストを生成する--*/
	cmdList_ = nullptr;
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator_.Get(), nullptr, IID_PPV_ARGS(&cmdList_));

	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	return true;
}

bool Graphics::CreateSyncObjects()
{
	// 初期値0でFenceを作る
	HRESULT hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	// FenceのSignalを待つためのイベント作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);

	return true;
}

bool Graphics::CreateViewport()
{
	// クライアント領域のサイズと一緒にして画面全体に表示
	viewport_.Width = float(width_);
	viewport_.Height = float(height_);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	Logger::Write("viewport");

	return true;
}

bool Graphics::CreateScissorRect()
{
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect_.left = 0;
	scissorRect_.right = LONG(width_);
	scissorRect_.top = 0;
	scissorRect_.bottom = LONG(height_);
	Logger::Write("scissorRect");

	return true;
}

void Graphics::InitFixFPS()
{
	// 現在時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

void Graphics::UpdateFixFPS()
{
	// 1/60秒にぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	// 1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 64.5f));

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	// 前回記録からの経過時間を取得
	const std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);
	
	// 1/60(より僅かに短い時間) 経ってない場合
	if (elapsed < kMinTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			// 1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	// 現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}
