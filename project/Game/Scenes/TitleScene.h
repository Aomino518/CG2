#pragma once
#include "SeekerEngine.h"
#include "BaseScene.h"

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
	// テクスチャ
	uint32_t texTitleLogo_;
	uint32_t texPressSpace_;

	// スプライト
	std::unique_ptr<Sprite> sprTitleLogo_;
	std::unique_ptr<Sprite> sprUiPressSpace_;

	// モデル
	std::unique_ptr<Entity3D> modelPlayer_;

	// カメラマネージャー
	std::unique_ptr<CameraManager> camMgr_;
};

