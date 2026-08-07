#pragma once
#include "ParticleEmitter.h"

class EmitterManager
{
public:
	static EmitterManager* GetInstance();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Shutdown();

	/// <summary>
	/// エミッター作成関数
	/// </summary>
	/// <param name="name">名称</param>
	/// <param name="config">コンフィグ</param>
	/// <param name="count">1回にスポーンする数</param>
	/// <param name="frequency">スポーン間隔</param>
	void CreateEmitter(const std::string& name,
		const ParticleConfig& config,
		uint32_t count = 10,
		float frequency = 0.1f);

	/// <summary>
	/// エミッター削除関数
	/// </summary>
	/// <param name="name"></param>
	void RemoveEmitter(const std::string& name);

	void DrawDebug();

	void DrawImgui(const std::string& name);

	// Getter関数
	ParticleEmitter* GetEmitter(const std::string& name);
	std::vector<ParticleEmitter*> GetAllEmitters() const;
	const std::unordered_map<std::string, std::unique_ptr<ParticleEmitter>>&GetEmitterMap() const;
	uint32_t GetEmitterCount() const;
private:
	// メンバ関数
	EmitterManager() = default;
	~EmitterManager() = default;
	EmitterManager(const EmitterManager&) = delete;
	EmitterManager& operator=(const EmitterManager&) = delete;

	// メンバ変数
	std::unordered_map<std::string, std::unique_ptr<ParticleEmitter>> emitters_;
};

