#include "Player.h"
#include "Vector3.h"
#include "SceneIncludes.h"

void Player::Init(const Vector3& position) {
	modelPlayer_ = std::make_unique<Entity3D>();
	modelPlayer_->Init();
	modelPlayer_->SetModel("player");
	rot_ = modelPlayer_->GetRotate();

	bullets_.resize(bulletMax_);
	for (int i = 0; i < bulletMax_; i++) {
		bullets_[i] = std::make_unique<Bullet>();
		bullets_[i]->Init(position);
	}

	modelPlayer_->SetTranslate(position);
	sphere_ = { position, 3.0f };
}

void Player::Update() {
	auto camMgr = CameraManager::GetInstance();

	if (isAlive_) {
		Move();
		Vector3 pos = modelPlayer_->GetTranslate();
		sphere_.center = pos;
	}

	for(auto& bullet : bullets_) {
		bullet->Update();
	}

	if (!camMgr->GetIsDebug()) {
		Camera* camera = camMgr->GetActiveCamera();

		modelPlayer_->SetCamera(camera);
	}

	modelPlayer_->Update();
}

void Player::Draw() {

	for (auto& bullet : bullets_) {
		if (bullet->GetIsShot()) {
			bullet->Draw();
		}
	}

	if (isAlive_) {
		modelPlayer_->Draw();
	}
}

void Player::Move()
{
	auto camMgr = CameraManager::GetInstance();
	if (camMgr->GetIsDebug()) {
		return;
	}

	inputDir = { 0.0f, 0.0f };
	rot_ = { 0.0f, 0.0f, 0.0f };

	// 移動入力
	if (Input::GetInstance()->IsPress(DIK_W)) {
		inputDir.y += 1.0f;
	}

	if (Input::GetInstance()->IsPress(DIK_S)) {
		inputDir.y -= 1.0f;
	}

	if (Input::GetInstance()->IsPress(DIK_A)) {
		inputDir.x -= 1.0f;
		rot_.z = 0.5f;
	}

	if (Input::GetInstance()->IsPress(DIK_D)) {
		inputDir.x += 1.0f;
		rot_.z = -0.5f;
	}

	modelPlayer_->SetRotate(rot_);

	// 入力方向正規化
	if (inputDir.x != 0.0f || inputDir.y != 0.0f) {
		float length = sqrtf(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
		inputDir.x /= length;
		inputDir.y /= length;

		velocity_.x += inputDir.x * acceleration;
		velocity_.y += inputDir.y * acceleration;
	}

	velocity_.x *= (1.0f - deceleration);
	velocity_.y *= (1.0f - deceleration);

	float speed = sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
	if (speed > maxSpeed) {
		velocity_.x = (velocity_.x / speed) * maxSpeed;
		velocity_.y = (velocity_.y / speed) * maxSpeed;
	}

	Vector3 pos = modelPlayer_->GetTranslate();
	pos.x += velocity_.x;
	pos.y += velocity_.y;
	// 移動範囲制限
	pos.x = std::clamp(pos.x, -33.0f, 33.0f);
	pos.y = std::clamp(pos.y, -20.0f, 20.0f);
	modelPlayer_->SetTranslate(pos);

	if (bulletCooldown_ > 0) {
		bulletCooldown_--;
	}

	for (auto& bullet : bullets_) {
		if (!bullet->GetIsShot()) {
			if (Input::GetInstance()->IsPress(DIK_SPACE)) {

				if (bulletCooldown_ <= 0) {
					if (!bullet->GetIsShot()) {
						bullet->SetPosition(pos);
						bullet->SetIsShot(true);
						bulletCooldown_ = 10;
						break;
					}
				}

			} else {
				bulletCooldown_ = 0;
			}
		}
	}
}
