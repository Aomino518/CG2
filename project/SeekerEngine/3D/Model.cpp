#include "Model.h"
#include "TextureManager.h"
#include <fstream>
#include "Logger.h"
#include <filesystem>
#include "Graphics.h"

void Model::Init(const std::string& directoryPath, const std::string& filename, const std::string& path)
{
	cmdList_ = Graphics::GetInstance()->GetCmdList();
	LoadObjFile(directoryPath, filename, path);
	CreateBufferResources();
	MaterialInit();
	modelData_.material.textureIndex = TextureManager::GetInstance()->Load(modelData_.material.textureFilePath);
	textureSrvHandleGPU_ = TextureManager::GetInstance()->GetGPUHandle(modelData_.material.textureIndex);
}

void Model::Draw()
{
	cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	cmdList_->IASetIndexBuffer(&indexBufferView_);
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定する。
	cmdList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	cmdList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	// 描画 (DrawCall)。
	cmdList_->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);
	Graphics::GetInstance()->AddDrawCallCount();
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	// 必要な変数宣言とファイルを開く
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // 開けなかったら止める

	// ファイルを読み、MaterialDataを構築
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

void Model::LoadObjFile(const std::string& directoryPath, const std::string& filename, const std::string& extension)
{
	Assimp::Importer importer;
	std::string	filePath = directoryPath + "/" + filename + "/" + filename + "." + extension;
	Logger::Write(filePath);
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	assert(scene->HasMeshes()); // メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線がないMeshは非対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは非対応
		uint32_t baseVertex = uint32_t(modelData_.vertices.size());

		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			aiVector3D& p = mesh->mVertices[i];
			aiVector3D& n = mesh->mNormals[i];
			aiVector3D& uv = mesh->mTextureCoords[0][i];

			VertexData vertex{};
			vertex.position = { -p.x, p.y, p.z, 1.0f };
			vertex.normal = { -n.x, n.y, n.z };
			vertex.texcoord = { uv.x, uv.y };

			modelData_.vertices.push_back(vertex);
		}

		// Meshの中身の解析を行う
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート
			
			for (uint32_t i = 0; i < 3; ++i) {
				modelData_.indices.push_back(
					baseVertex + face.mIndices[i]
				);
			}
		}
	}

	// Materialの解析を行う
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			std::string fullPath;
			fullPath = directoryPath + "/" + filename + "/" + textureFilePath.C_Str();
			Logger::Write("Trying to load texture from: " + fullPath);
			modelData_.material.textureFilePath = fullPath;
		}
	}

	modelData_.rootNode = ReadNode(scene->mRootNode);
}

void Model::CreateBufferResources()
{
	// 頂点リソース
	vertexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	Logger::Write("モデルのVertexResource生成完了");

	indexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(uint32_t) * modelData_.indices.size());
	D3D12_INDEX_BUFFER_VIEW indexBufferViewModel{};
	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData_.indices.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	Logger::Write("モデルのindexResource生成完了");

	// モデル用の頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);
	vertexData_ = nullptr;
	Logger::Write("モデルのVertexData書き込み完了");

	// モデル用の頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	std::memcpy(indexData_, modelData_.indices.data(), sizeof(uint32_t) * modelData_.indices.size());
	indexResource_->Unmap(0, nullptr);
	indexData_ = nullptr;
	Logger::Write("モデルのindexDataに書き込み完了");
}

void Model::MaterialInit()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->uvTransform = MakeIdentity4x4();
	materialData_->enableLighting = true;
	materialData_->shininess = 32.0f;
	Logger::Write("MaterialResourceの作成完了");
}

Node Model::ReadNode(aiNode* node)
{
	Node result;
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;
	aiLocalMatrix.Transpose();
	result.localMatrix.m[0][0] = aiLocalMatrix[0][0];
	result.localMatrix.m[0][1] = aiLocalMatrix[0][1];
	result.localMatrix.m[0][2] = aiLocalMatrix[0][2];
	result.localMatrix.m[0][3] = aiLocalMatrix[0][3];
	result.localMatrix.m[1][0] = aiLocalMatrix[1][0];
	result.localMatrix.m[1][1] = aiLocalMatrix[1][1];
	result.localMatrix.m[1][2] = aiLocalMatrix[1][2];
	result.localMatrix.m[1][3] = aiLocalMatrix[1][3];
	result.localMatrix.m[2][0] = aiLocalMatrix[2][0];
	result.localMatrix.m[2][1] = aiLocalMatrix[2][1];
	result.localMatrix.m[2][2] = aiLocalMatrix[2][2];
	result.localMatrix.m[2][3] = aiLocalMatrix[2][3];
	result.localMatrix.m[3][0] = aiLocalMatrix[3][0];
	result.localMatrix.m[3][1] = aiLocalMatrix[3][1];
	result.localMatrix.m[3][2] = aiLocalMatrix[3][2];
	result.localMatrix.m[3][3] = aiLocalMatrix[3][3];

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}
