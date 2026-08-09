#pragma once
#include "SeekerEngine.h"
#include "SceneIncludes.h"

class Fade {
public:
	// フェードの状態
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};

	void Init();

	void Update();

	void Draw();

	// フェード開始
	void Start(Status status, float duration);

	// フェード停止
	void Stop();

	// フェード終了判定
	bool IsFinished() const;

private:
	std::unique_ptr<Sprite> sprite_;

	uint32_t tex_;

	// 現在のフェードの状態
	Status status_ = Status::None;

	// フェードの持続時間
	float duration_ = 0.0f;
	// 経過時間カウンター
	float counter_ = 0.0f;
};

