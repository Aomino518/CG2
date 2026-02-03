#pragma once
#include "SeekerEngine.h"

enum class EnemyPattern {
	Straight,
	SinWave,
	ZigZag,
	SlowFast
};

class Enemy
{
public:
	void Init(EnemyPattern pattern, const Vector3& position);

	void Update();

	void Draw();

	Vector3 GetPosition() const { return model_->GetTranslate(); }
	bool GetIsAlive() const { return isAlive_; }
	Sphere GetSphere() const { return sphere_; }

private:
	Transform transform_{};
	std::unique_ptr<Entity3D> model_;

	float speed = 0.3f;
	float amplitude = 0.5f;
	float theta = 0.0f;
	bool isAlive_ = false;
	int switchDirTimer = 30;
	float dir = 1.0f;
	EnemyPattern pattern_;
	Sphere sphere_;
};

