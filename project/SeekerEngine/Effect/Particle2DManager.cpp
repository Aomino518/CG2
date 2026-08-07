#include "Particle2DManager.h"
#include "Graphics.h"
#include "Logger.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "CreateResorceUtils.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "Emitter2DManager.h"
#include "WorldField2DManager.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif
#include <numbers>

constexpr float kBaseWidth = 1280.0f;
constexpr float kBaseHeight = 720.0f;

Particle2DManager* Particle2DManager::GetInstance()
{
	static Particle2DManager instance;
	return &instance;
}

void Particle2DManager::Init(DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature)
{
	graphics_ = Graphics::GetInstance();
	rootSignature_ = rootSignature;
	// ランダムエンジンの初期化
	std::random_device seed;
	randomEngine_ = std::mt19937(seed());

	CreateGraphicPipline(dxcCompiler);
	CreateSprite();

	cmdList_ = Graphics::GetCmdList();

	width_ = Graphics::GetWidth();
	height_ = Graphics::GetHeight();
}

void Particle2DManager::Update()
{
	width_ = Graphics::GetWidth();
	height_ = Graphics::GetHeight();

	float scaleX = float(width_) / kBaseWidth;
	float scaleY = float(height_) / kBaseHeight;

	for (auto& [name, group] : particleGroups) {
		group.instanceCount = 0;
		for (auto particleIterator = group.particles.begin(); particleIterator != group.particles.end(); ) {
			// 移動と時間の更新
			if (particleIterator->lifeTime <= particleIterator->currentTime) {
				particleIterator = group.particles.erase(particleIterator);
				continue;
			}

			auto emitters = Emitter2DManager::GetInstance()->GetAllEmitters();
			auto worldFields = WorldField2DManager::GetInstance()->GetAllWorldFields();
			// ワールドフィールドの更新
			Vector3 totalAcceleration{ 0.0f, 0.0f, 0.0f };
			for (const auto& field : worldFields) {
				if (!field->GetIsActive()) {
					continue;
				}

				if (IsCollision(field->GetWorldAABB(), particleIterator->transform.translate)) {
					Vector2 acceleration = field->GetAcceleration();
					totalAcceleration.x = totalAcceleration.x + acceleration.x;
					totalAcceleration.y = totalAcceleration.y + acceleration.y;
				}
			}

			// ローカルフィールドの更新
			for (const auto& emitter : emitters) {
				Transform transform = emitter->GetTransform();
				auto& field = emitter->GetLocalField();
				if (!field.GetIsActive()) {
					continue;
				}

				if (IsCollision(field.GetWorldAABB({ transform.translate.x, transform.translate.y }), particleIterator->transform.translate)) {
					Vector2 acceleration = field.GetAcceleration();
					totalAcceleration.x = totalAcceleration.x + acceleration.x;
					totalAcceleration.y = totalAcceleration.y + acceleration.y;
					totalAcceleration.z = 0.0f;
				}
			}
			particleIterator->velocity = particleIterator->velocity + totalAcceleration * kDeltaTime;
			
			particleIterator->transform.translate = particleIterator->transform.translate + particleIterator->velocity * kDeltaTime;
			particleIterator->transform.rotate = particleIterator->transform.rotate + particleIterator->rotateVelocity * kDeltaTime;
			particleIterator->currentTime += kDeltaTime;

			// 色の補間
			float t = particleIterator->currentTime / particleIterator->lifeTime;
			t = std::clamp(t, 0.0f, 1.0f);
			Vector4 color = particleIterator->startColor * (1.0f - t) + particleIterator->endColor * t;

			// スケールの補間
			Vector3 scale = particleIterator->startScale * (1.0f - t) + particleIterator->endScale * t;
			particleIterator->transform.scale = scale;

			Vector3 scaledTranslate = particleIterator->transform.translate;
			scaledTranslate.x = particleIterator->transform.translate.x * scaleX;
			scaledTranslate.y = particleIterator->transform.translate.y * scaleY;

			Vector3 scaledScale = particleIterator->transform.scale;
			scaledScale.x = particleIterator->transform.scale.x * scaleX;
			scaledScale.y = particleIterator->transform.scale.y * scaleY;

			Matrix4x4 worldMatrix = MakeAffineMatrix(scaledScale, particleIterator->transform.rotate, scaledTranslate);
			Matrix4x4 viewMatrix = MakeIdentity4x4();
			Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(width_), float(height_), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			
			if (group.instanceCount < kNumMaxInstance_) {
				// 書き込み  
				group.instanceData[group.instanceCount].World = worldMatrix;
				group.instanceData[group.instanceCount].WVP = worldViewProjectionMatrix;
				group.instanceData[group.instanceCount].color = color;
				++group.instanceCount;
			}

			++particleIterator;
		}
	}
}

void Particle2DManager::Draw()
{
	for (auto& [name, group] : particleGroups) {
		cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
		cmdList_->SetPipelineState(GetPso(group.blendMode_));
		cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		SrvManager::GetInstance()->PreDraw();
		cmdList_->IASetVertexBuffers(0, 1, &vbView_);
		cmdList_->IASetIndexBuffer(&ibView_);
		cmdList_->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

		if (group.instanceCount == 0) {
			continue;
		}

		cmdList_->SetGraphicsRootDescriptorTable(2, group.textureSrvHandle);
		SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(3, group.srvIndexCount);
		cmdList_->DrawIndexedInstanced(6, group.instanceCount, 0, 0, 0);
	}
}

void Particle2DManager::Shutdown()
{
	for (auto& [name, group] : particleGroups) {
		SrvManager::GetInstance()->Free(group.srvIndexCount);

		group.instanceResource.Reset();
	}
	particleGroups.clear();
	rootSignature_.Reset();
	psoParticle2D_.Reset();
	cmdList_.Reset();
	vsBlob_.Reset();
	psBlob_.Reset();
	psoCache_.clear();
	materialResource.Reset();
	vertexBufferResource_.Reset();
	indexBufferResource_.Reset();
	Logger::Write("Particle2DManager Shutdown");
}

void Particle2DManager::Emit(const std::string name, const ParticleConfig& config, const Vector3& position, uint32_t count)
{
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);

	auto it = particleGroups.find(name);
	assert(it != particleGroups.end());

	ParticleGroup2D& group = it->second;

	for (uint32_t i = 0; i < count; ++i) {
		Particle particle{};
		Vector2 spawnOffset{};

		if (config.shape == SpawnShape::Box) {
			spawnOffset.x = RandomRange(randomEngine_, config.boxMin.x, config.boxMax.x);
			spawnOffset.y = RandomRange(randomEngine_, config.boxMin.y, config.boxMax.y);
		} else if (config.shape == SpawnShape::Sphere) {
			float theta = RandomRange(randomEngine_, 0.0f, 2.0f * std::numbers::pi_v<float>);
			float r = RandomRange(randomEngine_, 0.0f, config.sphereRadius);

			spawnOffset.x = r * cosf(theta);
			spawnOffset.y = r * sinf(theta);
		}

		// Vector2 を Vector3 に変換（Z は 0.0f に設定）
		particle.transform.translate = { position.x + spawnOffset.x, position.y + spawnOffset.y, 0.0f };
		particle.transform.rotate = RandomRange(randomEngine_, config.minRotate, config.maxRotate);
		particle.transform.rotate.x = 0.0f;
		particle.transform.rotate.y = 0.0f;
		particle.transform.scale = RandomRange(randomEngine_, config.startScaleMin, config.startScaleMax);
		particle.transform.scale.z = 0.0f;
		particle.rotateVelocity = RandomRange(randomEngine_, config.minRotateVelocity, config.maxRotateVelocity);
		particle.rotateVelocity.x = 0.0f;
		particle.rotateVelocity.y = 0.0f;
		particle.velocity = RandomRange(randomEngine_, config.minVelocity, config.maxVelocity);
		particle.velocity.z = 0.0f;
		particle.color = { distColor(randomEngine_), distColor(randomEngine_), distColor(randomEngine_), 1.0f };
		particle.startColor = RandomRange(randomEngine_, config.startColorMin, config.startColorMax);
		particle.endColor = RandomRange(randomEngine_, config.endColorMin, config.endColorMax);
		particle.color = particle.startColor;
		particle.startScale = RandomRange(randomEngine_, config.startScaleMin, config.startScaleMax);
		particle.endScale = RandomRange(randomEngine_, config.endScaleMin, config.endScaleMax);
		particle.lifeTime = RandomRange(randomEngine_, config.minLifeTime, config.maxLifeTime);
		particle.currentTime = 0.0f;

		group.particles.push_back(particle);
	}
}

void Particle2DManager::DrawParticleGroup2DImGui(const std::string & name)
{
#ifdef USE_IMGUI
	auto& group = Particle2DManager::GetInstance()->GetGroup(name);

	ImGui::Text("Group: %s", name.c_str());

	int mode = static_cast<int>(group.blendMode_);
	const char* modes[] = { "None", "Normal", "Add", "Sub", "Mul", "Screen" };
	if (ImGui::Combo("BlendMode", &mode, modes, IM_ARRAYSIZE(modes))) {
		Particle2DManager::GetInstance()->SetBlendMode(name, static_cast<BlendMode>(mode));
	}
#endif
}

ID3D12PipelineState* Particle2DManager::GetPso(BlendMode mode)
{
	auto it = psoCache_.find(mode);
	if (it != psoCache_.end()) {
		return it->second.Get();
	}

	mode_ = mode;
	RebuildPso();

	psoCache_[mode] = psoParticle2D_;

	return psoParticle2D_.Get();;
}

ParticleGroup2D& Particle2DManager::GetGroup(const std::string& name)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle2D group not found");
	return it->second;
}

BlendMode Particle2DManager::GetBlendMode(const std::string& name)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle2D group not found");
	return it->second.blendMode_;
}

uint32_t Particle2DManager::GetTotalParticleCount() const
{
	uint32_t totalCount = 0;
	for (const auto& [name, group] : particleGroups) {
		totalCount += static_cast<uint32_t>(group.particles.size());
	}
	return totalCount;
}

uint32_t Particle2DManager::GetParticleGroupCount() const
{
	return static_cast<uint32_t>(particleGroups.size());
}

void Particle2DManager::SetBlendMode(const std::string& name, BlendMode mode)
{
	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		Logger::Write(Logger::LogLevel::Warning, name + " is not Particle2D");
		return;
	}

	it->second.blendMode_ = mode;
}

void Particle2DManager::RebuildPso()
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = false;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	InputLayout inputLayout;
	D3D12_INPUT_LAYOUT_DESC layout = inputLayout.CreateInputLayout2D();

	blendDesc_ = CreateBlendDesc(mode_);

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	builder.Init(graphics_);
	psoDesc = builder.CreatePsoDesc(
		rootSignature_.Get(),
		layout,
		vsBlob_,
		psBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	psoParticle2D_ = builder.BuildPso(psoDesc);
}

void Particle2DManager::CreateParticleGroup(const std::string& name, uint32_t textureId)
{
	// 同名がないかチェック
	if (particleGroups.find(name) != particleGroups.end()) {
		Logger::Write(Logger::LogLevel::Warning, "Particle2D group already exists: " + name);
		return;
	}

	ParticleGroup2D group{};
	group.textureIndex = textureId;
	group.textureSrvHandle = TextureManager::GetInstance()->GetGPUHandle(textureId);
	group.particles.clear();
	group.instanceCount = 0;
	group.srvIndexCount = SrvManager::GetInstance()->Allocate();
	size_t bufferSize = sizeof(ParticleForGPU) * kNumMaxInstance_;
	group.instanceResource = CreateBufferResource(Graphics::GetDevice(), bufferSize);
	group.instanceData = nullptr;
	group.instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&group.instanceData));

	for (uint32_t i = 0; i < kNumMaxInstance_; ++i) {
		group.instanceData[i].WVP = MakeIdentity4x4();
		group.instanceData[i].World = MakeIdentity4x4();
		group.instanceData[i].color = { 1,1,1,1 };
	}

	Logger::Write("Particle2D instancing buffer created");

	SrvManager::GetInstance()->CreateSRVforStructuredBuffer(
		group.srvIndexCount,
		group.instanceResource.Get(),
		kNumMaxInstance_,
		sizeof(ParticleForGPU)
	);

	Logger::Write("Particle2D instancing SRV created");

	particleGroups.emplace(name, std::move(group));
}

void Particle2DManager::RemoveParticleGroup(const std::string& name)
{
	auto it = particleGroups.find(name);
	if (it == particleGroups.end()) {
		return;
	}

	// SRVとリソースを解放
	SrvManager::GetInstance()->Free(it->second.srvIndexCount);
	it->second.instanceResource.Reset();
	// particlesのコンテナは自動的に破棄
	particleGroups.erase(it);

	Logger::Write("Particle2D group removed: " + name);
}

json Particle2DManager::SaveToJson(const std::string& name) const
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle2D group not found");

	return json{
		{"blendMode", it->second.blendMode_},
	};
}

void Particle2DManager::LoadFromJson(const json& j, const std::string& name)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end() && "Particle2D group not found");

	if (j.contains("blendMode")) {
		it->second.blendMode_ = static_cast<BlendMode>(j.value("blendMode", it->second.blendMode_));
	}
}

void Particle2DManager::CreateGraphicPipline(DxcCompiler& dxcCompiler)
{
	depthStencilDesc_ = {};
	// DepthStencilStateの設定
	depthStencilDesc_.DepthEnable = true;
	// 書き込みします
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// InputLayout
	InputLayout inputLayout;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc2D{};
	inputLayoutDesc2D = inputLayout.CreateInputLayout2D();

	blendDesc_ = CreateBlendDesc(mode_);

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	vsBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Particle2D.VS.hlsl", L"vs_6_0");
	psBlob_ = dxcCompiler.CompileShader(L"resources/hlsl/Particle2D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	// 2D用
	PsoBuilder builder;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2D{};
	builder.Init(graphics_);
	psoDesc2D = builder.CreatePsoDesc(
		rootSignature_,
		inputLayoutDesc2D,
		vsBlob_,
		psBlob_,
		blendDesc_,
		rasterizerDesc,
		depthStencilDesc_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	psoParticle2D_ = builder.BuildPso(psoDesc2D);
	Logger::Write("pipelineState生成完了");
}

void Particle2DManager::CreateSprite()
{
	VertexData vertices[4] = {
	{{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	{{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	{{ 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	{{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	};

	uint32_t indices[6] = { 0, 1, 2, 1, 3, 2 };

	// BufferSize
	size_t vertexBufferSize = sizeof(vertices);
	size_t indexBufferSize = sizeof(indices);

	// Resourceの作成
	vertexBufferResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(VertexData) * 4);
	indexBufferResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(uint32_t) * 6);

	// VertexDataに割り当て
	VertexData* vertexData = nullptr;
	vertexBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices, sizeof(vertices));
	vertexBufferResource_->Unmap(0, nullptr);
	Logger::Write("VertexDataに割り当て完了");

	// リソースの先頭のアドレスから使う
	vbView_.BufferLocation = vertexBufferResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vbView_.SizeInBytes = UINT(vertexBufferSize);
	// 1頂点あたりのサイズ
	vbView_.StrideInBytes = sizeof(VertexData);
	Logger::Write("VertexBufferView生成完了");

	// IndexDataに割り当て
	uint32_t* indexData = nullptr;
	indexBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices, sizeof(indices));
	indexBufferResource_->Unmap(0, nullptr);
	Logger::Write("indexDataに割り当て完了");

	// リソースの先頭のアドレスから使う
	ibView_.BufferLocation = indexBufferResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	ibView_.SizeInBytes = UINT(indexBufferSize);
	// インデックスはuint32_tとする
	ibView_.Format = DXGI_FORMAT_R32_UINT;
	Logger::Write("IndexBufferView生成完了");

	// マテリアルリソースを作る
	materialResource = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// SpriteはLightingしないのでfalseを設定する
	materialData->color = Vector4(1, 1, 1, 1);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
}

float Particle2DManager::RandomRange(std::mt19937& engine, float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(engine);
}

Vector2 Particle2DManager::RandomRange(std::mt19937& engine, const Vector2& min, const Vector2& max)
{
	return {
	   RandomRange(engine, min.x, max.x),
	   RandomRange(engine, min.y, max.y),
	};
}

Vector3 Particle2DManager::RandomRange(std::mt19937& engine, const Vector3& min, const Vector3& max)
{
	return {
	   RandomRange(engine, min.x, max.x),
	   RandomRange(engine, min.y, max.y),
	   RandomRange(engine, min.z, max.z)
	};
}

Vector4 Particle2DManager::RandomRange(std::mt19937& engine, const Vector4& min, const Vector4& max)
{
	return {
		RandomRange(engine, min.x, max.x),
		RandomRange(engine, min.y, max.y),
		RandomRange(engine, min.z, max.z),
		RandomRange(engine, min.w, max.w)
	};
}
