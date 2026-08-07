#pragma once
#include "Model.h"
#include <unordered_map>
#include <string>
#include <cassert>
#include <wrl.h>
#include <vector>
#include <map>
#include "Logger.h"

class ModelManager
{
public:
	// シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	// 終了
	void Shutdown();

	void Init();

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadModel(const std::string& filePath);

	/// <summary>
	/// モデル検索
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <returns>モデル</returns>
	Model* FindModel(const std::string& filePath);

	// Getter
	bool GetIsModelLighting() const { return this->isModelLighting_; }

	// Setter
	void SetIsLighting(bool isLighting);

private:
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	bool isModelLighting_ = true;

	/// <summary>
	/// 文字列を分離する関数
	/// </summary>
	/// <param name="str">文字列</param>
	/// <param name="del">区切る対象文字</param>
	/// <returns>分離した文字列</returns>
	std::vector<std::string> Split(std::string str, char del);

	// モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models_;
};

