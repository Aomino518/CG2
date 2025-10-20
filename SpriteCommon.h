#pragma once

class SpriteCommon {
public:
	void Init();

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックパイプラインの作成
	void CreateGraphicPipeline();
};
