#include "Bullet.h"
#include "Enemy.h"
#include "SceneIncludes.h"

void Bullet::Init(const Vector3& position) {
	model_ = std::make_unique<Entity3D>();
	model_->Init();
	model_->SetModel("bullet");
	model_->SetTranslate(position);
	sphere_ = {position, 1.0f};
}

void Bullet::Update() {
	auto camMgr = CameraManager::GetInstance();

	if (isShot_) {
		Vector3 pos = model_->GetTranslate();
		pos.z += speed_;
		sphere_.center = pos;
		model_->SetTranslate(pos);
		
		if (pos.z >= 100.0f) {
			isShot_ = false;
		}
	}

	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		model_->SetCamera(camera);
	}

	model_->Update();
}

void Bullet::Draw() {
	if (isShot_) {
		model_->Draw();
	}
}

bool Bullet::BulletIsCollision(const Enemy& enemy) const
{
	return IsCollision(sphere_, enemy.GetSphere());
}
