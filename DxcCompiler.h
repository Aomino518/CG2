#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <format>
#include <assert.h>
#include "Logger.h"
#include "StringUtil.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

class DxcCompiler
{
public:
	void Init();
	void ShaderBlob(
		const std::wstring& vertexFilePath,
		const wchar_t* vertexProfile,
		const std::wstring& pixelFilePath,
		const wchar_t* pixelProfile);

	// CompileShader関数
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		// CompilerするShaderファイルへのパス
		const std::wstring& filePath,
		// Compilerに使用するProfile
		const wchar_t* profile,
		// 初期化で生成したものを3つ
		Microsoft::WRL::ComPtr<IDxcUtils>& dxcUtils,
		Microsoft::WRL::ComPtr<IDxcCompiler3>& dxcCompiler,
		Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler);

	IDxcUtils* GetDxcUtils() const { return dxcUtils_.Get(); }
	IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler_.Get(); }
	IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_.Get(); }
	IDxcBlob* GetVertexShaderBlob() const { return vertexShaderBlob_.Get(); }
	IDxcBlob* GetPixelShaderBlob() const { return pixelShaderBlob_.Get(); }

private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;
};

