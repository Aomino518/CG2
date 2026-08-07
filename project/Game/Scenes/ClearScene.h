#pragma once
#include "BaseScene.h"
#include "SeekerEngine.h"
#include "Fade.h"

class ClearScene : public BaseScene
{
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// 終了
	void Shutdown() override;

	const char* GetSceneName() const override { return "CLEAR"; }

private:
	// シーンフェーズ
	enum struct ScenePhase {
		FADEIN,
		MAIN,
		FADEOUT
	};

	// メンバ関数
	void UpdateImGui(); // ImGuiの更新処理

	// テクスチャ
	uint32_t texUiGameClear_;
	uint32_t texUiBackTitle_;

	// スプライト
	std::unique_ptr<Sprite> sprGameClear_;
	std::unique_ptr<Sprite> sprUiSpaceBackTitle_;

	// モデル
	std::unique_ptr<Entity3D> modelSkydome_;

	// カメラマネージャー
	std::unique_ptr<CameraManager> camMgr_;

	// シーンフェーズ
	ScenePhase phase_ = ScenePhase::FADEIN;

	// クラス
	Fade fade_;
};

