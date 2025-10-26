#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "Graphics.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <deque>

class TextureManager
{
public:
	static void Init(Graphics* graphics);

	static uint32_t Load(const std::string& filePath);

	static void Shutdown();

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t textureId);

	static void ClearIntermediate();

private:
	struct TextureData {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};
	
	static ID3D12Device* device_;
	static ID3D12GraphicsCommandList* cmdList_;
	static ID3D12DescriptorHeap* srvHeap_;
	static uint32_t descriptorSize_;
	static uint32_t textureCount_;

	static std::unordered_map<std::string, uint32_t> pathToId_;
	static std::vector<TextureData> textures_;
	static Microsoft::WRL::ComPtr<ID3D12Resource> intermediaste_;
	static std::deque<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediasteResource_;

	// 内部関数
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	static DirectX::ScratchImage LoadFromFile(const std::string& filePath);
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	[[nodiscard]]
	static Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
			Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
			const DirectX::ScratchImage& mipImages);
};