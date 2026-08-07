#include "TextureManager.h"
#include "Graphics.h"
#include "Logger.h"
#include <format>
#include "StringUtil.h"
#include "CreateResorceUtils.h"
#include "SrvManager.h"

using namespace Microsoft::WRL;

TextureManager* TextureManager::GetInstance()
{
	static TextureManager instance;
	return &instance;
}

void TextureManager::Init()
{
	device_ = Graphics::GetInstance()->GetDevice();
	cmdList_ = Graphics::GetInstance()->GetCmdList();
	descriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t TextureManager::Load(const std::string& filePath)
{
	if (pathToId_.contains(filePath)) {
		return pathToId_[filePath];
	}

	if (textureCount_ >= SrvManager::kMaxSRVCount) {
		Logger::Write(std::format("[TextureManager] SRV limit exceeded ({}/{})",
			textureCount_, SrvManager::kMaxSRVCount));
		assert(false && "SRV Descriptor Heap limit exceeded!");
		return 0;
	}

	DirectX::ScratchImage mipImages = LoadFromFile(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(metadata);
	intermediaste_ = UploadTextureData(textureResource, mipImages);
	intermediasteResource_.push_back(intermediaste_);

	uint32_t srvIndex = SrvManager::GetInstance()->Allocate();
	SrvManager::GetInstance()->CreateSRVforTexture2D(srvIndex, textureResource.Get(), metadata.format, static_cast<UINT>(metadata.mipLevels));

	// 登録してID発行
	uint32_t id = static_cast<uint32_t>(textures_.size());
	textures_.push_back({ textureResource, srvIndex, metadata});
	pathToId_[filePath] = id;

	return id;
}

void TextureManager::Shutdown()
{
	textures_.clear();
	pathToId_.clear();

	intermediaste_.Reset();
	intermediasteResource_.clear();

	device_ = nullptr;
	cmdList_ = nullptr;
	Logger::Write("TextureManager Shutdown");
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(uint32_t textureId)
{
	assert(textureId < textures_.size());
	uint32_t srvIndex = textures_[textureId].srvIndex;
	return SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);
}

void TextureManager::ClearIntermediate()
{
	// Fence後に呼ぶ
	intermediasteResource_.clear();
}

void TextureManager::Unload(uint32_t textureId)
{
	TextureData tex = textures_[textureId];
	// SRVインデックスの解放
	SrvManager::GetInstance()->Free(tex.srvIndex);
	// DirectXのresourceを解放
	tex.resource.Reset();
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex)
{
	// 範囲外指定違反チェック
	assert(textureIndex < textures_.size() && "テクスチャ番号が範囲外です");

	// テクスチャデータの参照を取得
	const TextureData& textureData = textures_[textureIndex];

	return textureData.metadata;
}

DirectX::ScratchImage TextureManager::LoadFromFile(const std::string& filePath)
{
	// テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	if (FAILED(hr)) {
		Logger::Write(Logger::LogLevel::Error, "Failed to load texture");
		assert(SUCCEEDED(hr));
	}

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	if (FAILED(hr)) {
		Logger::Write(Logger::LogLevel::Error, "Failed to generate mipmaps for: " + filePath);
		assert(SUCCEEDED(hr));
	}

	// ミップマップ付きのデータを返す
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata)
{
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数、普段使ってるのは2次元

	// 利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // プロセッサの近くに配置

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr;
	hr = device_->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST, // 初回のResourceState。Textureは基本読むだけ
		nullptr, // Clear最適値。使わないのでnullpr
		IID_PPV_ARGS(&resource) // 作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));
	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device_, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediasteResource = CreateBufferResource(device_, intermediateSize);
	UpdateSubresources(cmdList_, texture.Get(), intermediasteResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	// Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_RTEADへResourceStateへ変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	cmdList_->ResourceBarrier(1, &barrier);
	return intermediasteResource;
}