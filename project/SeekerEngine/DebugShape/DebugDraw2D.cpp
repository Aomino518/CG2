#define NOMINMAX
#include "DebugDraw2D.h"
#include "Graphics.h"
#include "Logger.h"
#include <assert.h>
#include <numbers>

DebugDraw2D* DebugDraw2D::GetInstance() {
	static DebugDraw2D instance;
	return &instance;
}

void DebugDraw2D::Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature) {
	graphics_ = Graphics::GetInstance();
	rootSignature_ = rootSignature;
	CreateGraphicPipeline(dxcCompiler);
	cmdList_ = Graphics::GetCmdList();
	CreateVertexBuffer();
	CreateTransformationMatrixResource();
}

void DebugDraw2D::Shutdown() {
	psoDebug2DWire_.Reset();
	psoDebug2DSolid_.Reset();
	vsDebug2DBlob_.Reset();
	psDebug2DBlob_.Reset();
	rootSignature_.Reset();
	graphics_ = nullptr;
	psoCache_.clear();
	cmdList_.Reset();
	Logger::Write("DebugDraw2D Shutdown");
}

void DebugDraw2D::CreateVertexBuffer()
{
	// Wire用
	vertexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(DebugVertex) * kMaxVertexCount);

	// VertexBufferViewを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(DebugVertex) * kMaxVertexCount;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(DebugVertex);
	Logger::Write("VertexBufferView生成完了");

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// Solid用
	solidVertexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(DebugVertex) * kMaxVertexCount);

	// VertexBufferViewを作成する
	// リソースの先頭のアドレスから使う
	solidVertexBufferView_.BufferLocation = solidVertexResource_->GetGPUVirtualAddress();
	solidVertexBufferView_.SizeInBytes = sizeof(DebugVertex) * kMaxVertexCount;
	// 1頂点あたりのサイズ
	solidVertexBufferView_.StrideInBytes = sizeof(DebugVertex);
	Logger::Write("SolidVertexBufferView生成完了");

	solidVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&solidVertexData_));
}

void DebugDraw2D::CreateTransformationMatrixResource()
{
	transformationMatrixResource_ = CreateBufferResource(
		Graphics::GetDevice(),
		sizeof(TransformationMatrix)
	);

	transformationMatrixResource_->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&transformationMatrixData_)
	);

	transformationMatrixData_->WVP = MakeIdentity4x4();
}

void DebugDraw2D::DrawWire()
{
	cmdList_->SetPipelineState(psoDebug2DWire_.Get());
	cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	cmdList_->DrawInstanced(vertexCount_, 1, 0, 0);
	Graphics::GetInstance()->AddDrawCallCount();
}

void DebugDraw2D::DrawSolid()
{
	cmdList_->SetPipelineState(psoDebug2DSolid_.Get());
	cmdList_->IASetVertexBuffers(0, 1, &solidVertexBufferView_);
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList_->DrawInstanced(solidVertexCount_, 1, 0, 0);
	Graphics::GetInstance()->AddDrawCallCount();
}

void DebugDraw2D::DrawPolygon(const Vector2& point1, const Vector2& point2, const Vector2& point3, const Vector4& color)
{
	solidVertexData_[solidVertexCount_ + 0].position = { point1.x, point1.y, 1.0f };
	solidVertexData_[solidVertexCount_ + 0].color = color;

	solidVertexData_[solidVertexCount_ + 1].position = { point2.x, point2.y, 1.0f };
	solidVertexData_[solidVertexCount_ + 1].color = color;

	solidVertexData_[solidVertexCount_ + 2].position = { point3.x, point3.y, 1.0f };
	solidVertexData_[solidVertexCount_ + 2].color = color;

	solidVertexCount_ += 3;
}

void DebugDraw2D::DrawLine(const Vector2& start, const Vector2& end, const Vector4& color) {
	if (vertexCount_ + 2 > kMaxVertexCount) {
		Logger::Write("DebugDraw2D vertex buffer overflow"); 
		assert(false && "DebugDraw2D vertex buffer overflow");
		return;
	}

	vertexData_[vertexCount_ + 0].position = { start.x, start.y, 0.0f, 1.0f };
	vertexData_[vertexCount_ + 0].color = color;

	vertexData_[vertexCount_ + 1].position = { end.x, end.y, 0.0f, 1.0f };
	vertexData_[vertexCount_ + 1].color = color;

	vertexCount_ += 2;
}

void DebugDraw2D::DrawCircleWire(const Vector2& center, float radiusX, float radiusY, const Vector4& color)
{
	const uint32_t kSegmentCount = 32;
	const float step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSegmentCount);

	for (uint32_t i = 0; i < kSegmentCount; ++i) {
		float angle0 = step * static_cast<float>(i);
		float angle1 = step * static_cast<float>((i + 1) % kSegmentCount);

		Vector2 p0 = {
			center.x + std::cos(angle0) * radiusX,
			center.y + std::sin(angle0) * radiusY
		};

		Vector2 p1 = {
			center.x + std::cos(angle1) * radiusX,
			center.y + std::sin(angle1) * radiusY
		};

		DrawLine(p0, p1, color);
	}
}

void DebugDraw2D::DrawCircleSolid(const Vector2& center, float radiusX, float radiusY, const Vector4& color)
{
	const uint32_t kSegmentCount = 32;
	const float step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSegmentCount);

	uint32_t needVertex = kSegmentCount * 3;

	if (solidVertexCount_ + needVertex > kMaxVertexCount) {
		Logger::Write("DebugDraw2D solid vertex overflow");
		assert(false);
		return;
	}

	for (uint32_t i = 0; i < kSegmentCount; ++i) {
		float angle0 = step * static_cast<float>(i);
		float angle1 = step * static_cast<float>((i + 1) % kSegmentCount);

		Vector2 p0 = {
			center.x + std::cos(angle0) * radiusX,
			center.y + std::sin(angle0) * radiusY
		};

		Vector2 p1 = {
			center.x + std::cos(angle1) * radiusX,
			center.y + std::sin(angle1) * radiusY
		};

		// center
		solidVertexData_[solidVertexCount_ + 0].position = { center.x, center.y, 0.0f, 1.0f };
		solidVertexData_[solidVertexCount_ + 0].color = color;

		// p0
		solidVertexData_[solidVertexCount_ + 1].position = { p0.x, p0.y, 0.0f, 1.0f };
		solidVertexData_[solidVertexCount_ + 1].color = color;

		// p1
		solidVertexData_[solidVertexCount_ + 2].position = { p1.x, p1.y, 0.0f, 1.0f };
		solidVertexData_[solidVertexCount_ + 2].color = color;

		solidVertexCount_ += 3;
	}
}

void DebugDraw2D::DrawBoxWire(const Vector2& center, const Vector2& halfSize, const Vector4& color)
{
	float lightTopX = center.x + halfSize.x;
	float lightTopY = center.y - halfSize.y;
	float leftBottomX = center.x - halfSize.x;
	float leftBottomY = center.y + halfSize.y;
	Vector2 p0 = center - halfSize;
	Vector2 p1 = { lightTopX, lightTopY };
	Vector2 p2 = { leftBottomX, leftBottomY };
	Vector2 p3 = center + halfSize;
	DrawLine(p0, p1, color);
	DrawLine(p1, p3, color);
	DrawLine(p3, p2, color);
	DrawLine(p2, p0, color);
}

void DebugDraw2D::DrawBoxSolid(const Vector2& center, const Vector2& halfSize, const Vector4& color)
{
	float lightTopX = center.x + halfSize.x;
	float lightTopY = center.y - halfSize.y;
	float leftBottomX = center.x - halfSize.x;
	float leftBottomY = center.y + halfSize.y;
	Vector2 p0 = center - halfSize;
	Vector2 p1 = { lightTopX, lightTopY };
	Vector2 p2 = { leftBottomX, leftBottomY };
	Vector2 p3 = center + halfSize;

	solidVertexData_[solidVertexCount_ + 0].position = { p0.x, p0.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 0].color = color;

	solidVertexData_[solidVertexCount_ + 1].position = { p1.x, p1.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 1].color = color;

	solidVertexData_[solidVertexCount_ + 2].position = { p2.x, p2.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 2].color = color;

	solidVertexData_[solidVertexCount_ + 3].position = { p1.x, p1.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 3].color = color;

	solidVertexData_[solidVertexCount_ + 4].position = { p3.x, p3.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 4].color = color;

	solidVertexData_[solidVertexCount_ + 5].position = { p2.x, p2.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 5].color = color;

	solidVertexCount_ += 6;
}

void DebugDraw2D::DrawRectWire(const Vector2& leftTop, const Vector2& size, const Vector4& color)
{
	float lightTopX = leftTop.x + size.x;
	float lightTopY = leftTop.y;
	float leftBottomX = leftTop.x;
	float leftBottomY = leftTop.y + size.y;
	Vector2 p0 = leftTop;
	Vector2 p1 = { lightTopX, lightTopY };
	Vector2 p2 = { leftBottomX, leftBottomY };
	Vector2 p3 = leftTop + size;
	DrawLine(p0, p1, color);
	DrawLine(p1, p3, color);
	DrawLine(p3, p2, color);
	DrawLine(p2, p0, color);
}

void DebugDraw2D::DrawRectSolid(const Vector2& leftTop, const Vector2& size, const Vector4& color)
{
	float lightTopX = leftTop.x + size.x;
	float lightTopY = leftTop.y;
	float leftBottomX = leftTop.x;
	float leftBottomY = leftTop.y + size.y;
	Vector2 p0 = leftTop;
	Vector2 p1 = { lightTopX, lightTopY };
	Vector2 p2 = { leftBottomX, leftBottomY };
	Vector2 p3 = leftTop + size;

	solidVertexData_[solidVertexCount_ + 0].position = { p0.x, p0.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 0].color = color;

	solidVertexData_[solidVertexCount_ + 1].position = { p1.x, p1.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 1].color = color;

	solidVertexData_[solidVertexCount_ + 2].position = { p2.x, p2.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 2].color = color;

	solidVertexData_[solidVertexCount_ + 3].position = { p1.x, p1.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 3].color = color;

	solidVertexData_[solidVertexCount_ + 4].position = { p3.x, p3.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 4].color = color;

	solidVertexData_[solidVertexCount_ + 5].position = { p2.x, p2.y, 0.0f, 1.0f };
	solidVertexData_[solidVertexCount_ + 5].color = color;

	solidVertexCount_ += 6;
}

void DebugDraw2D::DrawAABB2DWire(const Vector2& position, const AABB2D& aabb, const Vector4& color)
{
	Vector2 minPos = aabb.min + position;
	Vector2 maxPos = aabb.max + position;

	Vector3 min = {
		std::min(minPos.x, maxPos.x),
		std::min(minPos.y, maxPos.y)
	};

	Vector3 max = {
		std::max(minPos.x, maxPos.x),
		std::max(minPos.y, maxPos.y)
	};

	Vector2 v0 = { min.x, min.y };
	Vector2 v1 = { max.x, min.y };
	Vector2 v2 = { max.x, max.y };
	Vector2 v3 = { min.x, max.y };

	DrawLine(v0, v1, color);
	DrawLine(v1, v2, color);
	DrawLine(v2, v3, color);
	DrawLine(v3, v0, color);
}

void DebugDraw2D::DrawAABB2DSolid(const Vector2& position, const AABB2D& aabb, const Vector4& color)
{
	Vector2 minPos = aabb.min + position;
	Vector2 maxPos = aabb.max + position;

	Vector3 min = {
		std::min(minPos.x, maxPos.x),
		std::min(minPos.y, maxPos.y)
	};

	Vector3 max = {
		std::max(minPos.x, maxPos.x),
		std::max(minPos.y, maxPos.y)
	};

	Vector2 v0 = { min.x, min.y };
	Vector2 v1 = { max.x, min.y };
	Vector2 v2 = { max.x, max.y };
	Vector2 v3 = { min.x, max.y };

	DrawPolygon(v0, v1, v3, color);
	DrawPolygon(v1, v2, v3, color);
}

void DebugDraw2D::Update() {
	vertexCount_ = 0;
	solidVertexCount_ = 0;
	Matrix4x4 worldMatrix = MakeIdentity4x4();
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(
		0.0f,
		0.0f,
		static_cast<float>(Graphics::GetWidth()),
		static_cast<float>(Graphics::GetHeight()),
		0.1f,
		100.0f
	);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
}

void DebugDraw2D::Draw() {
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetGraphicsRootConstantBufferView(0, transformationMatrixResource_->GetGPUVirtualAddress());
	DrawWire();
	DrawSolid();
}

void DebugDraw2D::CreateGraphicPipeline(DxcCompiler dxcCompiler) {
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = false;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescDebug2D{};
	InputLayout inputLayout;
	inputLayoutDescDebug2D = inputLayout.CreateInputLayoutDebug2D();

	// BlendStateの設定
	// すべての色要素を書き込む
	blendDesc_ = CreateBlendDesc(mode_);
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

	// Shaderをコンパイルする
	vsDebug2DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/DebugShape2D.VS.hlsl", L"vs_6_0");
	psDebug2DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/DebugShape2D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	PsoBuilder builder;
	builder.Init(graphics_);
	// Wire用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2D{};
	psoDesc2D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDescDebug2D,
		vsDebug2DBlob_,
		psDebug2DBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	);

	psoDebug2DWire_ = builder.BuildPso(psoDesc2D);
	Logger::Write("PSODebug2DWire生成完了");

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDescSolid{};
	// 裏面(時計回り)を表示しない
	rasterizerDescSolid.CullMode = D3D12_CULL_MODE_NONE;

	rasterizerDescSolid.FillMode = D3D12_FILL_MODE_SOLID;

	// Solid用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2DSolid{};
	psoDesc2DSolid = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDescDebug2D,
		vsDebug2DBlob_,
		psDebug2DBlob_,
		blendDesc_,
		rasterizerDescSolid,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	psoDebug2DSolid_ = builder.BuildPso(psoDesc2DSolid);
	Logger::Write("PSODebug2DSolid生成完了");
}