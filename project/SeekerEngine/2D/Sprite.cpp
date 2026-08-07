#include "Sprite.h"
#include "SpriteCommon.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

constexpr float kBaseWidth = 1280.0f;
constexpr float kBaseHeight = 720.0f;

void Sprite::Init() {
	cmdList_ = Graphics::GetCmdList();
	mode_ = SpriteCommon::GetInstance()->GetBlendMode();

	// VertexResourceの作成
	vertexResource = CreateBufferResource(Graphics::GetDevice(), sizeof(VertexData) * 4);
	
	// IndexResourceの作成
	indexResource = CreateBufferResource(Graphics::GetDevice(), sizeof(uint32_t) * 6);
	
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
	materialResource = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// SpriteはLightingしないのでfalseを設定する
	materialData->color = Vector4(1, 1, 1, 1);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	// TransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = CreateBufferResource(Graphics::GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書きこんでおく
	transformationMatrixData->World = MakeIdentity4x4();
	transformationMatrixData->WVP = MakeIdentity4x4();
}

void Sprite::Update()
{
	uint32_t width = Graphics::GetWidth();
	uint32_t height = Graphics::GetHeight();

	float scaleX = float(width) / kBaseWidth;
	float scaleY = float(height) / kBaseHeight;

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

	const DirectX::TexMetadata& metaData = TextureManager::GetInstance()->GetMetaData(textureIndex_);

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

	Vector3 scaledTranslate = transform_.translate;
	scaledTranslate.x = transform_.translate.x * scaleX;
	scaledTranslate.y = transform_.translate.y * scaleY;

	Vector3 scaledScale = transform_.scale;
	scaledScale.x = transform_.scale.x * scaleX;
	scaledScale.y = transform_.scale.y * scaleY;

	// Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(scaledScale, transform_.rotate, scaledTranslate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(width), float(height), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
	transformationMatrixData->WVP = worldViewProjectionMatrix;
}

void Sprite::Draw()
{
	assert(textureSrvHandleGPU_.ptr != 0 && "Sprite texture not set!");
	SpriteCommon::GetInstance()->SetBlendMode(mode_);
	SpriteCommon::GetInstance()->ApplyPipeline();

	cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定

	cmdList_->IASetIndexBuffer(&indexBufferView);// IBV設定

	cmdList_->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	cmdList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	cmdList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

	cmdList_->DrawIndexedInstanced(6, 1, 0, 0, 0);

	Graphics::GetInstance()->AddDrawCallCount();
}

void Sprite::SetBlendMode(BlendMode mode)
{
	this->mode_ = mode;
	SpriteCommon::GetInstance()->SetBlendMode(mode);
}

void Sprite::Create(uint32_t textureId, const Vector2& pos, const Vector4& color, const Vector2& size)
{
	Init();
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
	transform_.translate.x += delta.x;
	transform_.translate.y += delta.y;
}

void Sprite::Rotate(float deltaAngle)
{
	transform_.rotate.z += deltaAngle;
}

void Sprite::Scale(float factor)
{
	transform_.scale.x *= factor;
	transform_.scale.y *= factor;
}

void Sprite::Scale(const Vector2& factor)
{
	transform_.scale.x *= factor.x;
	transform_.scale.y *= factor.y;
}

void Sprite::DrawImGui()
{
#ifdef USE_IMGUI
	Vector2 position = GetPosition();
	float rotation = GetRotation();
	Vector2 size = GetSize();
	Vector4 material = GetColor();

	bool changed = false;
	changed |= ImGui::DragFloat2("Position", reinterpret_cast<float*>(&position), 0.01f);
	changed |= ImGui::DragFloat("Rotation", &rotation, 0.1f);
	changed |= ImGui::DragFloat2("Scale", reinterpret_cast<float*>(&size), 0.01f, 0.0f);
	if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&material))) {
		SetMaterial(material);
	}

	if (changed) {
		SetPosition(position);
		SetRotation(rotation);
		SetSize(size);
	}

#endif
}

void Sprite::UpdateColorBlink(const Vector4& baseColor, const Vector4& blinkColor)
{
	colorBlinkFrame_++;

	if (colorBlinkFrame_ == 60) {
		colorBlinkFrame_ = 0;
	}

	float t = (sinf(colorBlinkFrame_ * colorBlinkSpeed_) + 1.0f) * 0.5f;

	Vector4 c;
	c.x = baseColor.x + (blinkColor.x - baseColor.x) * t;
	c.y = baseColor.y + (blinkColor.y - baseColor.y) * t;
	c.z = baseColor.z + (blinkColor.z - baseColor.z) * t;
	c.w = 1.0f;

	SetColor(c);
}

void Sprite::AdjustTextureSize()
{
	const DirectX::TexMetadata& metaData = TextureManager::GetInstance()->GetMetaData(textureIndex_);

	textureSize_.x = static_cast<float>(metaData.width);
	textureSize_.y = static_cast<float>(metaData.height);
	// 画像サイズをテクスチャのサイズに合わせる
	transform_.scale.x = textureSize_.x;
	transform_.scale.y = textureSize_.y;
}

json Sprite::SaveToJson() const {
	return json{
	   {"transform", TransformToJson(transform_)},
	   {"blendMode", static_cast<int>(mode_)},
	   {"material", ToJson(GetColor())}
	};
}

void Sprite::LoadFromJson(const json& j) {
	if (j.contains("transform")) {
		Transform transform{};
		TransformFromJson(j.at("transform"), transform);
		SetTransform(transform);
	}

	if (j.contains("blendMode")) {
		mode_ = static_cast<BlendMode>(j.at("blendMode").get<int>());
	}

	if (j.contains("material")) {
		Vector4 material{};
		FromJson(j.at("material"), material);
		SetMaterial(material);
	}
}