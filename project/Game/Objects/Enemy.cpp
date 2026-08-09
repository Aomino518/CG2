#include "Enemy.h"
#include <numbers>
#include "Vector3.h"

void Enemy::Init(EnemyPattern pattern, const Vector3& position) {
	model_ = std::make_unique<Entity3D>();
	model_->Init();
	model_->SetModel("enemy");
	model_->SetTranslate(position);
	isAlive_ = true;
	pattern_ = pattern;
	sphere_ = { position, 1.0f };
}

void Enemy::Update() {
	auto camMgr = CameraManager::GetInstance();

	if (!isMoveStop_) {
		transform_ = model_->GetTransform();
		if (isAlive_) {
			switch (pattern_) {
			case EnemyPattern::Straight:
				transform_.translate.z -= speed;

				break;
			case EnemyPattern::SinWave:
				theta += std::numbers::pi_v<float> / 60.0f;
				transform_.translate.x += sin(theta) * amplitude;
				transform_.translate.z -= speed;

				break;
			case EnemyPattern::ZigZag:
				switchDirTimer--;

				if (switchDirTimer <= 0) {
					dir = -dir;
					switchDirTimer = 30;
				}

				transform_.translate.x += dir * speed;
				transform_.translate.z -= speed;

				break;
			case EnemyPattern::SlowFast:
				// 徐々に加速
				speed += 0.02f;
				transform_.translate.z -= speed;

				break;
			}
		}

		sphere_.center = transform_.translate;
		model_->SetTranslate(transform_.translate);
	}

	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		model_->SetCamera(camera);
	}
	model_->Update();
}

void Enemy::Draw() {
	if (isAlive_) {
		model_->Draw();
	}
}