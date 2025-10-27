#include "Sprite.h"
#include "SpriteCommon.h"

void Sprite::Init(SpriteCommon* spriteCommon_) {
	this->spriteCommon = spriteCommon_;

	cmdList_ = spriteCommon_->GetGraphics()->GetCmdList();

	// VertexResourceの作成
	vertexResource = CreateBufferResource(spriteCommon_->GetGraphics()->GetDevice(), sizeof(VertexData) * 4);
	
	// IndexResourceの作成
	indexResource = CreateBufferResource(spriteCommon_->GetGraphics()->GetDevice(), sizeof(uint32_t) * 6);
	
	// VertexBufferViewを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	Logger::Write("VertexBufferView生成完了");

	// IndexBufferViewを作成する
	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	Logger::Write("IndexBufferView生成完了");

	// VertexDataに割り当て
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 矩形
	vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };
	Logger::Write("VertexDataに割り当て完了");

	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;
	Logger::Write("indexDataに割り当て完了");

	// マテリアルリソースを作る
	materialResource = CreateBufferResource(spriteCommon_->GetGraphics()->GetDevice(), sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// SpriteはLightingしないのでfalseを設定する
	materialData->color = Vector4(1, 1, 1, 1);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	// TransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = CreateBufferResource(spriteCommon_->GetGraphics()->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書きこんでおく
	transformationMatrixData->World = MakeIdentity4x4();
	transformationMatrixData->WVP = MakeIdentity4x4();
}

void Sprite::Update()
{
	Graphics* graphics = spriteCommon->GetGraphics();
	uint32_t width = graphics->GetWidth();
	uint32_t height = graphics->GetHeight();

	// translateの更新
	transform_.translate = { position_.x, position_.y, 0.0f };
	// rotationの更新
	transform_.rotate = { 0.0f,0.0f, rotation_ };
	// scaleの更新
	transform_.scale = { size_.x, size_.y, 1.0f };

	// アンカーポイントによる頂点再計算
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	// 左右回転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}

	// 上下回転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	vertexData[0].position = { left,  bottom, 0.0f, 1.0f }; // 左下
	vertexData[1].position = { left,  top,    0.0f, 1.0f }; // 左上
	vertexData[2].position = { right, bottom, 0.0f, 1.0f }; // 右下
	vertexData[3].position = { right, top,    0.0f, 1.0f }; // 右上

	const DirectX::TexMetadata& metaData = TextureManager::GetMetaData(textureIndex_);

	// UV座標を計算
	float tex_left = textureLeftTop_.x / metaData.width;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metaData.width;
	float tex_top = textureLeftTop_.y / metaData.height;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metaData.height;

	// 頂点データに書き込み
	vertexData[0].texcoord = { tex_left,  tex_bottom };
	vertexData[1].texcoord = { tex_left,  tex_top };
	vertexData[2].texcoord = { tex_right, tex_bottom };
	vertexData[3].texcoord = { tex_right, tex_top };

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));
	materialData->uvTransform = uvTransformMatrix;

	// Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(width), float(height), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
	transformationMatrixData->WVP = worldViewProjectionMatrix;
}

void Sprite::Draw()
{
	Update();

	assert(textureSrvHandleGPU_.ptr != 0 && "Sprite texture not set!");

	cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定

	cmdList_->IASetIndexBuffer(&indexBufferView);// IBV設定

	cmdList_->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	cmdList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	cmdList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

	cmdList_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::Create(SpriteCommon* common, uint32_t textureId, const Vector2& pos, const Vector4& color, const Vector2& size)
{
	Init(common);
	SetTexture(textureId);
	SetPosition(pos);
	SetColor(color);

	if (size.x == 0.0f && size.y == 0.0f) {
		AdjustTextureSize();
	} else {
		AdjustTextureSize();
		SetSize(size);
	}
}

void Sprite::Move(const Vector2& delta)
{
	position_.x += delta.x;
	position_.y += delta.y;
}

void Sprite::Rotate(float deltaAngle)
{
	rotation_ += deltaAngle;
}

void Sprite::Scale(float factor)
{
	size_.x *= factor;
	size_.y *= factor;
}

void Sprite::Scale(const Vector2& factor)
{
	size_.x *= factor.x;
	size_.y *= factor.y;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Sprite::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	// 頂点リソース用のヒープを設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr;
	hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void Sprite::AdjustTextureSize()
{
	const DirectX::TexMetadata& metaData = TextureManager::GetMetaData(textureIndex_);

	textureSize_.x = static_cast<float>(metaData.width);
	textureSize_.y = static_cast<float>(metaData.height);
	// 画像サイズをテクスチャのサイズに合わせる
	size_ = textureSize_;
}
