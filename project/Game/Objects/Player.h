#pragma once
#include "SeekerEngine.h"
#include "Matrix.h"
#include "Bullet.h"

class Player
{
public:
	// 初期化
	void Init(const Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// Getter
	Vector3 GetPosition() const { return modelPlayer_->GetTranslate(); }
	Vector3 GetRotate() const { return rot_; }
	bool GetIsAlive() const { return isAlive_; }
	Entity3D* GetModel() const { return modelPlayer_.get(); }
	std::vector<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }
	Sphere GetSphere() const { return sphere_; }

	// Setter
	void SetPosition(const Vector3& pos) { modelPlayer_->SetTranslate(pos); }
	void SetIsAlive(bool isAlive) { isAlive_ = isAlive; }

private:
	// モデル
	std::unique_ptr<Entity3D> modelPlayer_;

	// メンバ関数
	void Move();

	// メンバ変数
	// 速度
	Vector2 velocity_ = { 0.0f, 0.0f };
	// 加速度
	float acceleration = 0.05f;
	float deceleration = 0.05f;
	// 限界速度
	float maxSpeed = 0.5f;
	// 生存フラグ
	bool isAlive_ = true;
	// 入力方向
	Vector2 inputDir = { 0.0f, 0.0f };
	Vector3 rot_ = { 0.0f, 0.0f, 0.0f };
	int bulletCooldown_ = 0;
	int bulletMax_ = 10;
	// 当たり判定
	Sphere sphere_;

	// 弾丸
	std::vector<std::unique_ptr<Bullet>> bullets_;
};

