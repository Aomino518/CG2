#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "SrvManager.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <deque>

class TextureManager
{
public:
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();

	void Init();

	/// <summary>
	/// テクスチャのロード
	/// </summary>
	/// <param name="filePath">テクスチャのファイルパス</param>
	/// <returns>テクスチャハンドル</returns>
	uint32_t Load(const std::string& filePath);

	void Shutdown();

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t textureId);

	void ClearIntermediate();

	// テクスチャのアンロード
	void Unload(uint32_t textureId);

	// Getter関数
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);

private:
	static TextureManager* instance_;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	struct TextureData {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		DirectX::TexMetadata metadata;
	};
	
	ID3D12Device* device_ = nullptr;
	ID3D12GraphicsCommandList* cmdList_ = nullptr;
	uint32_t descriptorSize_ = 0;
	uint32_t textureCount_ = 1;

	std::unordered_map<std::string, uint32_t> pathToId_;
	std::vector<TextureData> textures_;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediaste_;
	std::deque<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediasteResource_;

	// 内部関数
	DirectX::ScratchImage LoadFromFile(const std::string& filePath);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
			Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
			const DirectX::ScratchImage& mipImages);
};