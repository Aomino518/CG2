#pragma once
#include "Particle2DEmitter.h"

class Emitter2DManager
{
public:
	static Emitter2DManager* GetInstance();

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
	Particle2DEmitter* GetEmitter(const std::string& name);
	std::vector<Particle2DEmitter*> GetAllEmitters() const;
	const std::unordered_map<std::string, std::unique_ptr<Particle2DEmitter>>& GetEmitterMap() const;
	uint32_t GetEmitterCount() const;

private:
	// メンバ関数
	Emitter2DManager() = default;
	~Emitter2DManager() = default;
	Emitter2DManager(const Emitter2DManager&) = delete;
	Emitter2DManager& operator=(const Emitter2DManager&) = delete;

	// メンバ変数
	std::unordered_map<std::string, std::unique_ptr<Particle2DEmitter>> emitters_;
};

