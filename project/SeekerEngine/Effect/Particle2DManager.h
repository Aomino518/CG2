#pragma once
#include "DxcCompiler.h"
#include "CreateResorceUtils.h"
#include "BlendStateUtils.h"
#include <random>
#include "AccelerationField.h"

struct ParticleGroup2D {
    // テクスチャ関連
    uint32_t textureIndex = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle{};
    // パーティクル本体
    std::list<Particle> particles;
    // インスタンシング
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
    uint32_t instanceCount = 0;
    uint32_t srvIndexCount = 0;
    ParticleForGPU* instanceData = nullptr;
    // ブレンドモード
    BlendMode blendMode_ = kBlendModeNone;
};

class Graphics;
class Particle2DManager
{
public:
	static Particle2DManager* GetInstance();

	void Init(DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature);
	void Update();
	void Draw();
	void Shutdown();
	void Emit(const std::string name,
        const ParticleConfig& config,
        const Vector3& position = { 0.0f, 0.0f },
        uint32_t count = 10);

    void DrawParticleGroup2DImGui(const std::string& name);

    // Getter関数
    ID3D12PipelineState* GetPso(BlendMode mode);
    ParticleGroup2D& GetGroup(const std::string& name);
    BlendMode GetBlendMode(const std::string& name);
    uint32_t GetkNumMaxInstance() { return kNumMaxInstance_; }
    uint32_t GetkNumInstance(const std::string& name) { return particleGroups[name].instanceCount; }
    uint32_t GetTotalParticleCount() const;
    uint32_t GetParticleGroupCount() const;

    // Setter関数
    void SetBlendMode(const std::string& name, BlendMode mode);

    void RebuildPso();
    // パーティクルグループの生成
    void CreateParticleGroup(const std::string& name, uint32_t textureId);
    // パーティクルグループの削除
    void RemoveParticleGroup(const std::string& name);

    // Json保存と読み込み
    json SaveToJson(const std::string& name) const;
    void LoadFromJson(const json& j, const std::string& name);

private:
	// メンバ関数
	Particle2DManager() = default;
	~Particle2DManager() = default;
	Particle2DManager(const Particle2DManager&) = delete;
	Particle2DManager& operator=(const Particle2DManager&) = delete;

    void CreateGraphicPipline(DxcCompiler& dxcCompiler);
    void CreateSprite();

    // ランダム関数
    float RandomRange(std::mt19937& engine, float min, float max);
    Vector2 RandomRange(std::mt19937& engine, const Vector2& min, const Vector2& max);
    Vector3 RandomRange(std::mt19937& engine, const Vector3& min, const Vector3& max);
    Vector4 RandomRange(std::mt19937& engine, const Vector4& min, const Vector4& max);

    // メンバ変数
    // パーティクルグループコンテナ
    std::unordered_map<std::string, ParticleGroup2D> particleGroups;
    Graphics* graphics_ = nullptr;
    // ランダムエンジンの変数
    std::mt19937 randomEngine_;
    static constexpr uint32_t kNumMaxInstance_ = 2000;
    const float kDeltaTime = 1.0f / 60.0f;

    //===========================
    // グラフィックパイプラインの変数
    //===========================
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> psoParticle2D_;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;

    D3D12_BLEND_DESC blendDesc_{};
    BlendMode mode_ = kBlendModeNone;
    std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;

    // リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferResource_;
    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    D3D12_INDEX_BUFFER_VIEW  ibView_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    Material* materialData = nullptr;

    uint32_t width_;
    uint32_t height_;
};

