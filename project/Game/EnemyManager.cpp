#include "EnemyManager.h"
#include "Player.h"
#include "Enemy.h"

void EnemyManager::Init(std::shared_ptr<Player> player) {
	frameCount_ = 0;
	enemies_.clear();
	player_ = player;
	// 出現スケジュール
	spawnList_ = {
		{60,   EnemyPattern::Straight, {-10, 0, 100} },
		{120,  EnemyPattern::Straight, {0, 0, 100}   },
		{180,  EnemyPattern::Straight, {10, 0, 100}  },
		{260,  EnemyPattern::SinWave,  {-12, 3, 100} },
		{340,  EnemyPattern::SinWave,  {12, 3, 100}  },
		{420,  EnemyPattern::ZigZag,   {-15, 0, 100} },
		{480,  EnemyPattern::ZigZag,   {15, 0, 100}  },
		{600,  EnemyPattern::SlowFast, {0, 10, 100}  },
		{750,  EnemyPattern::Straight, {-15, -8, 100}},
		{750,  EnemyPattern::Straight, {-5, -8, 100} },
		{750,  EnemyPattern::Straight, {5, -8, 100}  },
		{750,  EnemyPattern::Straight, {15, -8, 100} },
		{950,  EnemyPattern::ZigZag,   {-12, 0, 100} },
		{970,  EnemyPattern::ZigZag,   {0, 0, 100}   },
		{990,  EnemyPattern::ZigZag,   {12, 0, 100}  },
		{1150, EnemyPattern::SlowFast, {8, 10, 100}  },
		{1200, EnemyPattern::SlowFast, {-8, 10, 100} },
		{1400, EnemyPattern::SinWave,  {-20, 0, 100} },
		{1420, EnemyPattern::SinWave,  {-10, 0, 100} },
		{1440, EnemyPattern::SinWave,  {10, 0, 100}  },
		{1460, EnemyPattern::SinWave,  {20, 0, 100}  },
		{1650, EnemyPattern::Straight, {0, 10, 100}  },
		{1800, EnemyPattern::Straight, {-18, -6, 100}},
		{1800, EnemyPattern::Straight, {0, -6, 100}  },
		{1800, EnemyPattern::Straight, {18, -6, 100} },
		{1900, EnemyPattern::ZigZag,   {-12, 0, 100} },
		{1920, EnemyPattern::ZigZag,   {12, 0, 100}  },
		{2100, EnemyPattern::SlowFast, {0, 5, 100}   },
		{2180, EnemyPattern::SinWave,  {-10, 0, 100} },
		{2180, EnemyPattern::SinWave,  {10, 0, 100}  },
		{2400, EnemyPattern::Straight, {0, 0, 100}   },
	};
}

void EnemyManager::Update() {
	frameCount_++;

	if (frameCount_ >= 2700) {
		isFinished_ = true;
	}

	// 出現チェック
	for (auto it = spawnList_.begin(); it != spawnList_.end();) {
		if (frameCount_ >= it->spawnFrame) {
			SpawnEnemy(*it);
			it = spawnList_.erase(it);
		} else {
			it++;
		}
	}

	auto player = player_.lock();
	if (!player) {
		return;
	}

	auto& bullets = player->GetBullets();

	for (auto& bullet : bullets) {
		if (bullet->GetIsShot()) {
			for (auto it = enemies_.begin(); it != enemies_.end();) {
				Enemy* enemy = it->get();

				if (!enemy->GetIsAlive()) {
					++it;
					continue;
				}

				if (IsCollision(bullet->GetSphere(), enemy->GetSphere())) {
					bullet->SetIsShot(false);
					it = enemies_.erase(it);
					break;
				} else {
					it++;
				}
			}
		}
	}

	if (player->GetIsAlive()) {
		for (auto it = enemies_.begin(); it != enemies_.end();) {
			Enemy* enemy = it->get();

			if (!enemy->GetIsAlive()) {
				++it;
				continue;
			}

			if (IsCollision(player->GetSphere(), enemy->GetSphere())) {
				player->SetIsAlive(false);
				it = enemies_.erase(it);
			} else {
				it++;
			}
		}
	}

	// 敵の更新
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		(*it)->Update();

		if ((*it)->GetPosition().z < -50.0f) {
			it = enemies_.erase(it);
		} else {
			++it;
		}
	}
}

void EnemyManager::Draw() {
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
}

void EnemyManager::SpawnEnemy(const EnemySpawnData& data)
{
	auto enemy = std::make_unique<Enemy>();
	enemy->Init(data.enemyPattern, data.position);
	enemies_.push_back(std::move(enemy));
}
