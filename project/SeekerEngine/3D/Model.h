#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "CreateResorceUtils.h"
#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <algorithm>
#include "MathFunc.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	void Init(const std::string& directoryPath, const std::string& filename, const std::string& path);

	void Draw();

	/// <summary>
	/// mtlファイルの読み取り
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	/// <returns>マテリアルデータ</returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	void LoadObjFile(const std::string& directoryPath, const std::string& filename, const std::string& extension);

	Vector4& GetMaterial() const { return materialData_->color; }
	bool GetIsLighting() const { return materialData_->enableLighting; }
	ModelData GetRootNode() const { return modelData_; }

	void SetMaterial(const Vector4& material) { this->materialData_->color = material; }
	void SetIsLighting(const bool isLighting) { this->materialData_->enableLighting = isLighting; }

private:
	void CreateBufferResources();
	void MaterialInit();
	Node ReadNode(aiNode* node);

	// マテリアルデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;
	// モデルデータ
	ModelData modelData_ = {};

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};

