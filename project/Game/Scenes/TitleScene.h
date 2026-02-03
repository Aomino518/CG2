#pragma once
#include "SeekerEngine.h"
#include "BaseScene.h"
#include "Fade.h"

class TitleScene : public BaseScene
{
public:
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	void Shutdown() override;

private:
	// シーンフェーズ
	enum struct ScenePhase {
		FADEIN,
		MAIN,
		FADEOUT
	};

	// カメラフェーズ
	enum struct CameraPhase {
		BACK,
		LEFTSIDE,
		FRONT,
		RIGHTSIDE
	};

	// メンバ関数
	void UpdateImGui(); // ImGuiの更新処理
	void UpdateCamera();

	// メンバ変数
	float camSpeed_ = 0.1f;
	int frameCount_ = 0;

	// テクスチャ
	uint32_t texTitleLogo_;
	uint32_t texPressSpace_;
	uint32_t texMaou_;

	// スプライト
	std::unique_ptr<Sprite> sprTitleLogo_;
	std::unique_ptr<Sprite> sprUiPressSpace_;
	std::unique_ptr<Sprite> sprUiMaou_;

	// モデル
	std::unique_ptr<Entity3D> modelPlayer_;
	std::unique_ptr<Entity3D> modelSkydome_;

	// カメラマネージャー
	std::unique_ptr<CameraManager> camMgr_;
	Vector3 camPos_;
	Vector3 camRot_;

	// シーンフェーズ
	ScenePhase phase_ = ScenePhase::FADEIN;
	// カメラフェーズ
	CameraPhase camPhase_ = CameraPhase::BACK;

	// フェード
	Fade fade_;
};

