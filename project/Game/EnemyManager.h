#pragma once
#include "Enemy.h"

struct EnemySpawnData {
	int spawnFrame;
	EnemyPattern enemyPattern;
	Vector3 position;
};

class Player;
class Enemy;
class EnemyManager
{
public:
	// 初期化
	void Init(std::shared_ptr<Player> player);

	// 更新
	void Update();

	// 描画
	void Draw();

	bool GetIsFinished() const { return isFinished_; }

	void SetIsMoveStop(bool flag);

private:
	// メンバ関数
	void SpawnEnemy(const EnemySpawnData& data);

	// メンバ変数
	std::vector<std::unique_ptr<Enemy>> enemies_;
	std::vector<EnemySpawnData> spawnList_;
	std::weak_ptr<Player> player_;

	int frameCount_ = 0;
	bool isFinished_ = false;
	bool isStopFrame_ = false;
};

