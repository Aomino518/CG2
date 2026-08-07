#define NOMINMAX
#include "DebugDraw3D.h"
#include "Graphics.h"
#include "CameraManager.h"
#include "Logger.h"

DebugDraw3D* DebugDraw3D::GetInstance() {
	static DebugDraw3D instance;
	return &instance;
}

void DebugDraw3D::Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature) {
	graphics_ = Graphics::GetInstance();
	rootSignature_ = rootSignature;
	this->cameraManager_ = CameraManager::GetInstance();
	CreateGraphicPipeline(dxcCompiler);
	cmdList_ = Graphics::GetCmdList();
	CreateVertexBuffer();
	CreateTransformationMatrixResource();
}

void DebugDraw3D::Shutdown() {
	psoDebug3DWire_.Reset();
	psoDebug3DSolid_.Reset();
	vsDebug3DBlob_.Reset();
	psDebug3DBlob_.Reset();
	rootSignature_.Reset();
	graphics_ = nullptr;
	psoCache_.clear();
	cmdList_.Reset();
	Logger::Write("DebugDraw2D Shutdown");
}

void DebugDraw3D::Update() {
	wireVertexCount_ = 0;
	solidVertexCount_ = 0;
	Matrix4x4 worldMatrix = MakeIdentity4x4();
	// WVPMatrixを作る
	Matrix4x4 worldViewProjectionMatrix = worldMatrix;
	
	if (cameraManager_->GetIsDebug()) {
		auto* debugCamera = cameraManager_->GetDebugCamera();
		if (debugCamera) {
			worldViewProjectionMatrix = Multiply(worldMatrix, debugCamera->GetViewProjectionMatrix());
		}
	} else {
		auto* camera = cameraManager_->GetActiveCamera();
		if (camera) {
			worldViewProjectionMatrix = Multiply(worldMatrix, camera->GetViewProjectionMatrix());
		}
	}

	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
}

void DebugDraw3D::Draw() {
	cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList_->SetGraphicsRootConstantBufferView(0, transformationMatrixResource_->GetGPUVirtualAddress());
	DrawWire();
	DrawSolid();
}

void DebugDraw3D::DrawWire() {
	cmdList_->SetPipelineState(psoDebug3DWire_.Get());
	cmdList_->IASetVertexBuffers(0, 1, &wireVertexBufferView_);
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	cmdList_->DrawInstanced(wireVertexCount_, 1, 0, 0);
	Graphics::GetInstance()->AddDrawCallCount();
}

void DebugDraw3D::DrawSolid() {
	cmdList_->SetPipelineState(psoDebug3DSolid_.Get());
	cmdList_->IASetVertexBuffers(0, 1, &solidVertexBufferView_);
	cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList_->DrawInstanced(solidVertexCount_, 1, 0, 0);
	Graphics::GetInstance()->AddDrawCallCount();
}

void DebugDraw3D::DrawPolygon(const Vector3& point1, const Vector3& point2, const Vector3& point3, const Vector4& color)
{
	solidVertexData_[solidVertexCount_ + 0].position = { point1.x, point1.y, point1.z, 1.0f };
	solidVertexData_[solidVertexCount_ + 0].color = color;

	solidVertexData_[solidVertexCount_ + 1].position = { point2.x, point2.y, point2.z, 1.0f };
	solidVertexData_[solidVertexCount_ + 1].color = color;

	solidVertexData_[solidVertexCount_ + 2].position = { point3.x, point3.y, point3.z, 1.0f };
	solidVertexData_[solidVertexCount_ + 2].color = color;

	solidVertexCount_ += 3;
}

Vector4 DebugDraw3D::CreatePoint(const Vector3& center, const Vector3& radius, float lat, float lon)
{
	return {
			center.x + radius.x * std::cos(lat) * std::cos(lon),
			center.y + radius.y * std::sin(lat),
			center.z + radius.z * std::cos(lat) * std::sin(lon),
			1.0f
	};
}

void DebugDraw3D::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	if (wireVertexCount_ + 2 > kMaxVertexCount) {
		Logger::Write("DebugDraw2D vertex buffer overflow");
		assert(false && "DebugDraw2D vertex buffer overflow");
		return;
	}

	wireVertexData_[wireVertexCount_ + 0].position = { start.x, start.y, start.z, 1.0f };
	wireVertexData_[wireVertexCount_ + 0].color = color;

	wireVertexData_[wireVertexCount_ + 1].position = { end.x, end.y, end.z, 1.0f };
	wireVertexData_[wireVertexCount_ + 1].color = color;

	wireVertexCount_ += 2;
}

void DebugDraw3D::DrawBoxWire(const Vector3& center, const Vector3& halfSize, const Vector4& color)
{
	Vector3 leftTopBack = { center.x - halfSize.x, center.y + halfSize.y, center.z + halfSize.z };
	Vector3 lightTopBack = { center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z };
	Vector3 leftBottomBack = { center.x - halfSize.x, center.y - halfSize.y, center.z + halfSize.z };
	Vector3 lightBottomBack = { center.x + halfSize.x, center.y - halfSize.y, center.z + halfSize.z };
	Vector3 leftTopFront = { center.x - halfSize.x, center.y + halfSize.y, center.z - halfSize.z };
	Vector3 lightTopFront = { center.x + halfSize.x, center.y + halfSize.y, center.z - halfSize.z };
	Vector3 leftBottomFront = { center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z };
	Vector3 lightBottomFront = { center.x + halfSize.x, center.y - halfSize.y, center.z - halfSize.z };

	DrawLine(leftTopBack, lightTopBack, color);
	DrawLine(lightTopBack, lightBottomBack, color);
	DrawLine(lightBottomBack, leftBottomBack, color);
	DrawLine(leftBottomBack, leftTopBack, color);

	DrawLine(leftTopFront, lightTopFront, color);
	DrawLine(lightTopFront, lightBottomFront, color);
	DrawLine(lightBottomFront, leftBottomFront, color);
	DrawLine(leftBottomFront, leftTopFront, color);

	DrawLine(leftTopBack, leftTopFront, color);
	DrawLine(lightTopBack, lightTopFront, color);
	DrawLine(leftBottomBack, leftBottomFront, color);
	DrawLine(lightBottomBack, lightBottomFront, color);
}

void DebugDraw3D::DrawBoxSolid(const Vector3& center, const Vector3& halfSize, const Vector4& color)
{
	Vector3 leftTopBack = { center.x - halfSize.x, center.y + halfSize.y, center.z + halfSize.z };
	Vector3 lightTopBack = { center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z };
	Vector3 leftBottomBack = { center.x - halfSize.x, center.y - halfSize.y, center.z + halfSize.z };
	Vector3 lightBottomBack = { center.x + halfSize.x, center.y - halfSize.y, center.z + halfSize.z };
	Vector3 leftTopFront = { center.x - halfSize.x, center.y + halfSize.y, center.z - halfSize.z };
	Vector3 lightTopFront = { center.x + halfSize.x, center.y + halfSize.y, center.z - halfSize.z };
	Vector3 leftBottomFront = { center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z };
	Vector3 lightBottomFront = { center.x + halfSize.x, center.y - halfSize.y, center.z - halfSize.z };

	// 奥面
	DrawPolygon(leftTopBack, lightTopBack, leftBottomBack, color);
	DrawPolygon(lightTopBack, lightBottomBack, leftBottomBack, color);

	// 手前面
	DrawPolygon(leftTopFront, lightTopFront, leftBottomFront, color);
	DrawPolygon(lightTopFront, lightBottomFront, leftBottomFront, color);

	// 左面
	DrawPolygon(leftTopBack, leftTopFront, leftBottomBack, color);
	DrawPolygon(leftTopFront, leftBottomFront, leftBottomBack, color);

	// 右面
	DrawPolygon(lightTopBack, lightBottomBack, lightBottomFront, color);
	DrawPolygon(lightTopFront, lightTopBack, lightBottomFront, color);

	// 上面
	DrawPolygon(leftTopBack, lightTopBack, leftTopFront, color);
	DrawPolygon(lightTopBack, lightTopFront, leftTopFront, color);

	// 下面
	DrawPolygon(leftBottomBack, lightBottomBack, leftBottomFront, color);
	DrawPolygon(lightBottomBack, lightBottomFront, leftBottomFront, color);
}

void DebugDraw3D::DrawSphereWire(const Vector3& center, const Vector3& radius, const Vector4& color)
{
	uint32_t subdivision = 16;

	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / float(subdivision);
	const float kLatEvery = std::numbers::pi_v<float> / float(subdivision);

	for (uint32_t latIndex = 0; latIndex <= subdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			float lon0 = lonIndex * kLonEvery;
			float lon1 = (lonIndex + 1) * kLonEvery;

			Vector3 p0 = {
				center.x + radius.x * std::cos(lat) * std::cos(lon0),
				center.y + radius.y * std::sin(lat),
				center.z + radius.z * std::cos(lat) * std::sin(lon0)
			};

			Vector3 p1 = {
				center.x + radius.x * std::cos(lat) * std::cos(lon1),
				center.y + radius.y * std::sin(lat),
				center.z + radius.z * std::cos(lat) * std::sin(lon1)
			};

			DrawLine(p0, p1, color);
		}
	}

	for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
		float lon = lonIndex * kLonEvery;

		for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
			float lat0 = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
			float lat1 = -std::numbers::pi_v<float> / 2.0f + kLatEvery * (latIndex + 1);

			Vector3 p0 = {
				center.x + radius.x * std::cos(lat0) * std::cos(lon),
				center.y + radius.y * std::sin(lat0),
				center.z + radius.z * std::cos(lat0) * std::sin(lon)
			};

			Vector3 p1 = {
				center.x + radius.x * std::cos(lat1) * std::cos(lon),
				center.y + radius.y * std::sin(lat1),
				center.z + radius.z * std::cos(lat1) * std::sin(lon)
			};

			DrawLine(p0, p1, color);
		}
	}
}

void DebugDraw3D::DrawSphereSolid(const Vector3& center, const Vector3& radius, const Vector4& color)
{
	uint32_t subdivision = 16;
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / float(subdivision);
	const float kLatEvery = std::numbers::pi_v<float> / float(subdivision);

	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat0 = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
		float lat1 = -std::numbers::pi_v<float> / 2.0f + kLatEvery * (latIndex + 1);

		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			float lon0 = lonIndex * kLonEvery;
			float lon1 = (lonIndex + 1) * kLonEvery;

			Vector4 p0 = CreatePoint(center, radius, lat0, lon0);
			Vector4 p1 = CreatePoint(center, radius, lat1, lon0);
			Vector4 p2 = CreatePoint(center, radius, lat0, lon1);
			Vector4 p3 = CreatePoint(center, radius, lat1, lon1);

			if (solidVertexCount_ + 6 >= kMaxVertexCount) {
				return;
			}

			solidVertexData_[solidVertexCount_++] = { p0, color };
			solidVertexData_[solidVertexCount_++] = { p1, color };
			solidVertexData_[solidVertexCount_++] = { p2, color };

			solidVertexData_[solidVertexCount_++] = { p2, color };
			solidVertexData_[solidVertexCount_++] = { p1, color };
			solidVertexData_[solidVertexCount_++] = { p3, color };
		}
	}
}

void DebugDraw3D::DrawAABBWire(const Vector3& position, const AABB& aabb, const Vector4& color)
{
	Vector3 minPos = aabb.min + position;
	Vector3 maxPos = aabb.max + position;

	Vector3 min = {
		std::min(minPos.x, maxPos.x),
		std::min(minPos.y, maxPos.y),
		std::min(minPos.z, maxPos.z)
	};

	Vector3 max = {
		std::max(minPos.x, maxPos.x),
		std::max(minPos.y, maxPos.y),
		std::max(minPos.z, maxPos.z)
	};


	// 8頂点
	Vector3 v0 = { min.x, min.y, min.z };
	Vector3 v1 = { max.x, min.y, min.z };
	Vector3 v2 = { max.x, max.y, min.z };
	Vector3 v3 = { min.x, max.y, min.z };

	Vector3 v4 = { min.x, min.y, max.z };
	Vector3 v5 = { max.x, min.y, max.z };
	Vector3 v6 = { max.x, max.y, max.z };
	Vector3 v7 = { min.x, max.y, max.z };

	// 手前面
	DrawLine(v0, v1, color);
	DrawLine(v1, v2, color);
	DrawLine(v2, v3, color);
	DrawLine(v3, v0, color);

	// 奥面
	DrawLine(v4, v5, color);
	DrawLine(v5, v6, color);
	DrawLine(v6, v7, color);
	DrawLine(v7, v4, color);

	// 手前面と奥面をつなぐ
	DrawLine(v0, v4, color);
	DrawLine(v1, v5, color);
	DrawLine(v2, v6, color);
	DrawLine(v3, v7, color);
}

void DebugDraw3D::DrawAABBSolid(const Vector3& position, const AABB& aabb, const Vector4& color)
{
	Vector3 minPos = aabb.min + position;
	Vector3 maxPos = aabb.max + position;

	Vector3 min = {
		std::min(minPos.x, maxPos.x),
		std::min(minPos.y, maxPos.y),
		std::min(minPos.z, maxPos.z)
	};

	Vector3 max = {
		std::max(minPos.x, maxPos.x),
		std::max(minPos.y, maxPos.y),
		std::max(minPos.z, maxPos.z)
	};

	// 8頂点
	Vector3 v0 = { min.x, min.y, min.z };
	Vector3 v1 = { max.x, min.y, min.z };
	Vector3 v2 = { max.x, max.y, min.z };
	Vector3 v3 = { min.x, max.y, min.z };

	Vector3 v4 = { min.x, min.y, max.z };
	Vector3 v5 = { max.x, min.y, max.z };
	Vector3 v6 = { max.x, max.y, max.z };
	Vector3 v7 = { min.x, max.y, max.z };

	// 手前
	DrawPolygon(v3, v2, v0, color);
	DrawPolygon(v2, v1, v0, color);

	// 奥
	DrawPolygon(v7, v6, v4, color);
	DrawPolygon(v6, v5, v4, color);

	// 左面
	DrawPolygon(v7, v3, v4, color);
	DrawPolygon(v3, v0, v4, color);

	// 右面
	DrawPolygon(v2, v6, v1, color);
	DrawPolygon(v6, v5, v1, color);

	// 上面
	DrawPolygon(v7, v6, v3, color);
	DrawPolygon(v6, v2, v3, color);

	// 下面
	DrawPolygon(v1, v0, v5, color);
	DrawPolygon(v0, v4, v5, color);
}

void DebugDraw3D::CreateGraphicPipeline(DxcCompiler dxcCompiler) {
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = false;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescDebug3D{};
	InputLayout inputLayout;
	inputLayoutDescDebug3D = inputLayout.CreateInputLayoutDebug3D();

	// BlendStateの設定
	// すべての色要素を書き込む
	blendDesc_ = CreateBlendDesc(mode_);
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

	// Shaderをコンパイルする
	vsDebug3DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/DebugShape3D.VS.hlsl", L"vs_6_0");
	psDebug3DBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/DebugShape3D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	PsoBuilder builder;
	builder.Init(graphics_);
	// Wire用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc3D{};
	psoDesc3D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDescDebug3D,
		vsDebug3DBlob_,
		psDebug3DBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	);

	psoDebug3DWire_ = builder.BuildPso(psoDesc3D);
	Logger::Write("PSODebug3DWire生成完了");

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDescSolid{};
	// 裏面(時計回り)を表示しない
	rasterizerDescSolid.CullMode = D3D12_CULL_MODE_NONE;

	rasterizerDescSolid.FillMode = D3D12_FILL_MODE_SOLID;

	// Solid用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc3DSolid{};
	psoDesc3DSolid = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDescDebug3D,
		vsDebug3DBlob_,
		psDebug3DBlob_,
		blendDesc_,
		rasterizerDescSolid,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	psoDebug3DSolid_ = builder.BuildPso(psoDesc3DSolid);
	Logger::Write("PSODebug2DSolid生成完了");
}

void DebugDraw3D::CreateVertexBuffer() {
	// Wire用
	wireVertexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(DebugVertex) * kMaxVertexCount);

	// VertexBufferViewを作成する
	// リソースの先頭のアドレスから使う
	wireVertexBufferView_.BufferLocation = wireVertexResource_->GetGPUVirtualAddress();
	wireVertexBufferView_.SizeInBytes = sizeof(DebugVertex) * kMaxVertexCount;
	// 1頂点あたりのサイズ
	wireVertexBufferView_.StrideInBytes = sizeof(DebugVertex);
	Logger::Write("VertexBufferView生成完了");

	wireVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&wireVertexData_));

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

void DebugDraw3D::CreateTransformationMatrixResource() {
	// TransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書きこんでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
}