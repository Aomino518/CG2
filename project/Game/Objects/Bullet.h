#pragma once
#include "SeekerEngine.h"
#include "Vector3.h"
#include "SceneIncludes.h"

class Enemy;
class Bullet
{
public:
	// 初期化
	void Init(const Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// Getter
	Vector3 GetPosition() const { return model_->GetTranslate(); }
	bool GetIsShot() const { return isShot_; }
	Sphere GetSphere() const { return sphere_; }

	// Setter
	void SetPosition(const Vector3& pos) { model_->SetTranslate(pos); }
	void SetIsShot(bool isShot) { isShot_ = isShot; }

	bool BulletIsCollision(const Enemy& enemy) const;

private:
	// モデル
	std::unique_ptr<Entity3D> model_;

	// メンバ変数
	bool isShot_ = false;
	float speed_ = 3.0f;
	Sphere sphere_;
};

