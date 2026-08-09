#pragma once
#include "BaseScene.h"
#include "SeekerEngine.h"
#include "Fade.h"

class GameOverScene : public BaseScene
{
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// 終了
	void Shutdown() override;

	const char* GetSceneName() const override { return "GAMEOVER"; }

private:
	// シーンフェーズ
	enum struct ScenePhase {
		FADEIN,
		MAIN,
		FADEOUT
	};

	// テクスチャ
	uint32_t texUiGameOver_;
	uint32_t texUiBackTitle_;

	// スプライト
	std::unique_ptr<Sprite> sprGameOver_;
	std::unique_ptr<Sprite> sprUiSpaceBackTitle_;

	// モデル
	std::unique_ptr<Entity3D> modelSkydome_;

	// シーンフェーズ
	ScenePhase phase_ = ScenePhase::FADEIN;

	// クラス
	Fade fade_;
};

