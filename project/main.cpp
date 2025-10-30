#include "Application.h"
#include "Logger.h"
#include <format>
#include <dbghelp.h>
#include <strsafe.h>
#include <sstream>
#include "Matrix.h"
#include "DebugCamera.h"
#define _USE_MATH_DEFINES 
#include <math.h>
#include "StringUtil.h"
#include "Input.h"
#include "Sound.h"
#include "DxcCompiler.h"
#include "RootSignatureFactory.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include <algorithm>
#include <psapi.h>
#pragma comment(lib, "Dbghelp.lib")

#pragma region 構造体
struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct MaterialData {
	std::string textureFilePath;
};

// モデル関係の構造体
struct ModelData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	MaterialData material;
};

// 三つ組のキー
struct TripletKey {
	uint32_t v, vt, vn;
	bool operator == (const TripletKey&) const = default;
};

struct TripletHash {
	size_t operator()(const TripletKey& k) const noexcept {
		size_t h = 0;
		auto mix = [&](uint32_t x) {
			h ^= std::hash<uint32_t>{}(x)+0x9e3779b97f4a7c15ULL + (h << 5) + (h >> 2);
			};

		mix(k.v);
		mix(k.vt);
		mix(k.vn);
		return h;
	}
};

#pragma endregion

#pragma region 自作関数
void ShowMemoryUsage() {
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		// メモリ使用量をMB単位で計算
		double memoryUsageMB = pmc.WorkingSetSize / (1024.0 * 1024.0);

		ImGui::Text("Memory Usage: %.2f MB", memoryUsageMB);
	}
}

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻をなめに入れたファイルを作成、Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMilliseconds);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
	minidumpInfomation.ThreadId = threadId;
	minidumpInfomation.ExceptionPointers = exception;
	minidumpInfomation.ClientPointers = TRUE;
	// Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
	// 他に関連づけられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する。
	return EXCEPTION_EXECUTE_HANDLER;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	// 頂点リソース用のヒープを設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr;
	hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));
	return vertexResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height) {
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // Textureの幅
	resourceDesc.Height = height; // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥域 or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能フォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る
	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1,0f (最大値) でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr;
	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void WriteSphereVertices(const uint32_t subdivision, VertexData* vertexData) {
	const float kLonEvery = 2.0f * float(M_PI) / float(subdivision); // 経度
	const float kLatEvery = float(M_PI) / float(subdivision); // 緯度
	// 緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex <= subdivision; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;

		// 経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex <= subdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の経度kLonEvery

			Vector4 pos = {
				cos(lat) * cos(lon),
				sin(lat),
				cos(lat) * sin(lon),
				1.0f
			};

			// UV座標
			Vector2 uv = {
				lon / (2.0f * float(M_PI)),
				1.0f - (lat + float(M_PI) / 2.0f) / float(M_PI)
			};

			uint32_t index = latIndex * (subdivision + 1) + lonIndex;
			vertexData[index] = {
				pos,
				uv,
				Normalize(Vector3(pos.x, pos.y, pos.z))
			};

		}
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	// 必要な変数宣言とファイルを開く
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // 開けなかったら止める

	// ファイルを読み、MaterialDataを構築
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData; // 
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open());

	std::unordered_map<TripletKey, uint32_t, TripletHash> lut;

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		} else if (identifier == "f") {
			//VertexData triangle[3];

			struct FaceElm { uint32_t v, t, n; } f[3]{};

			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::replace(vertexDefinition.begin(), vertexDefinition.end(), '/', ' ');
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);

				v >> f[faceVertex].v >> f[faceVertex].t >> f[faceVertex].n;
				f[faceVertex].v--;
				f[faceVertex].t--;
				f[faceVertex].n--;
			}

			for (int order : {2, 1, 0}) {
				TripletKey key{ f[order].v, f[order].t, f[order].n };
				auto it = lut.find(key);
				uint32_t idx;
				if (it == lut.end()) {
					// 頂点生成
					Vector4 p = positions[key.v];
					Vector2 t = texcoords[key.vt];
					Vector3 n = normals[key.vn];

					p.x *= -1.0f;
					t.y = 1.0f - t.y;
					n.x *= -1.0f;

					VertexData vtx{ p, t, n };
					idx = (uint32_t)modelData.vertices.size();
					modelData.vertices.push_back(vtx);
					lut.emplace(key, idx);
				} else {
					idx = it->second;
				}
				modelData.indices.push_back(idx);
			}

		} else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一層にmtlは存在させるので、ディレクトリ名とファイル名を残す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
	CoInitializeEx(0, COINIT_MULTITHREADED);
	SetUnhandledExceptionFilter(ExportDump);
	Logger::Init();
	Logger::Write("アプリ開始");

	std::unique_ptr<Application> app = std::make_unique<Application>(1280, 720, L"CG2");
	app->Init();
	if (!app) {
		Logger::Write("App 初期化失敗");
		Logger::Shutdown();
		return false;
	}

	Graphics graphics;
	Input input;
	Sound xAudio2;
	DxcCompiler dxcCompiler;
	RootSignatureFactory rootSignatureFactory;
	InputLayout inputLayout;
	PsoBuilder psoBuilder;

	// graphicsの初期化
	graphics.Init(app->GetHWND(), app->GetWidth(), app->GetHeight(), true);

	TextureManager::Init(&graphics);

	// XAudio2の初期化
	xAudio2.Init();

	//DirectInput初期化
	input.Init(app.get());

	// デバイスの生成がうまくいかなかったので起動できない
	assert(graphics.GetDevice() != nullptr);
	// 初期化完了ログ
	Logger::Write("Complete Create D3D12Device!!!");

	//ID3D12GraphicsCommandList* cmdList_ = graphics.GetCmdList();

	// DxcCompilerの初期化
	dxcCompiler.Init();

	// RootSignature作成
	rootSignatureFactory.Init(&graphics);
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = rootSignatureFactory.CreateCommon();
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs2D = rootSignatureFactory.Create2D();

	std::unique_ptr<SpriteCommon> spriteCommon = std::make_unique<SpriteCommon>();
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();

	// スプライト共通部の作成
	spriteCommon->Init(dxcCompiler, rs2D.Get());

	Vector4 spriteMaterial = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 positoin = {0.0f, 0.0f};
	float rotation = 0.0f;
	//Vector2 size = {100.0f, 100.0f};

	uint32_t tHChecker = TextureManager::Load("resources/uvChecker.png");

#pragma region Texture読み込み
	// モデル読み込み
	/*ModelData modelData = LoadObjFile("resources", "axis.obj");
	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(graphics.GetDevice(), metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediasteResource2 = UploadTextureData(textureResource2, mipImages2, graphics.GetDevice(), cmdList_);*/
#pragma endregion

#pragma region SRVの設定
	// metadataを基にSRVを設定
	/*D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap = graphics.GetSRVHeap();
	uint32_t descSizeSRV = graphics.GetSRVDescriptorSize();

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvHeap, descSizeSRV, 0);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvHeap, descSizeSRV, 0);
	// 先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += graphics.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += graphics.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRVの生成
	graphics.GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	// 2つ目のSRVの作成
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvHeap, descSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvHeap, descSizeSRV, 2);
	// SRVの生成
	graphics.GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);*/
#pragma endregion

#pragma region リソース設定
	/*--モデル用のリソース設定--*/
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	/*Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(graphics.GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = Vector4(1, 1, 1, 1);
	materialData->uvTransform = MakeIdentity4x4();
	materialData->enableLighting = true;

	// 平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(graphics.GetDevice(), sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 初期化値
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 1.0f, 0.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(graphics.GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();

	//const uint32_t kSubdivision = 16; // 16分割

	//uint32_t vertexNum = (kSubdivision + 1) * (kSubdivision + 1);
	//uint32_t indexNum = kSubdivision * kSubdivision * 6;*/

	/*--Index用リソース作成--*/
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceModel = CreateBufferResource(graphics.GetDevice(), sizeof(uint32_t) * 6);
#pragma endregion

	// InputLayout
	/*D3D12_INPUT_LAYOUT_DESC inputLayoutDesc3D{};
	inputLayoutDesc3D = inputLayout.CreateInputLayout3D();

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vs3DBlob = dxcCompiler.CompileShader(L"resources/hlsl/Object3D.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> ps3DBlob = dxcCompiler.CompileShader(L"resources/hlsl/Object3D.PS.hlsl", L"ps_6_0");

	// PSOを生成する
	// 3D用
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc3D{};
	psoBuilder.Init(&graphics);
	psoDesc3D = psoBuilder.CreatePsoDesc(
		rootSignature,
		inputLayoutDesc3D,
		vs3DBlob,
		ps3DBlob,
		blendDesc,
		rasterizerDesc
	);

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso3D = psoBuilder.BuildPso(psoDesc3D);
	Logger::Write("PSO3D生成完了");*/

#pragma region モデル用の頂点リソース
	// 頂点リソース
	/*Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(graphics.GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	Logger::Write("VertexResource生成完了");

	// インデックスモデル用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferModel = CreateBufferResource(graphics.GetDevice(), sizeof(uint32_t) * modelData.indices.size());

	D3D12_INDEX_BUFFER_VIEW indexBufferViewModel{};
	// リソースの先頭のアドレスから使う
	indexBufferViewModel.BufferLocation = indexBufferModel->GetGPUVirtualAddress();
	indexBufferViewModel.SizeInBytes = UINT(sizeof(uint32_t) * modelData.indices.size());
	indexBufferViewModel.Format = DXGI_FORMAT_R32_UINT;
	Logger::Write("indexBufferViewModel生成完了");

	// モデル用の頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
	vertexResource->Unmap(0, nullptr);
	Logger::Write("VertexData生成完了");

	// モデル用の頂点リソースにデータを書き込む
	uint32_t* indexDataModel = nullptr;
	// 書き込むためのアドレスを取得
	indexBufferModel->Map(0, nullptr, reinterpret_cast<void**>(&indexDataModel));
	std::memcpy(indexDataModel, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());
	indexBufferModel->Unmap(0, nullptr);

	Logger::Write("indexDataModelに書き込み完了");*/
#pragma endregion

	//uint32_t index = 0;
	//uint32_t* indexDataModel = nullptr;
	//indexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&indexDataModel));
	//std::memcpy(indexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	/*for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t ld = lonIndex + latIndex * (kSubdivision + 1);
			uint32_t lt = lonIndex + (latIndex + 1) * (kSubdivision + 1);
			uint32_t rd = (lonIndex + 1) + latIndex * (kSubdivision + 1);
			uint32_t rt = (lonIndex + 1) + (latIndex + 1) * (kSubdivision + 1);

			indexDataModel[index++] = rt;
			indexDataModel[index++] = ld;
			indexDataModel[index++] = lt;

			indexDataModel[index++] = rd;
			indexDataModel[index++] = ld;
			indexDataModel[index++] = rt;
		}
	}*/

	// Transform変数を作る
	Transform transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	//Transform cameraTransform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };

	Transform uvTransformSprite = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};

	// 初期色 (RGBA)
	float modelColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 音声読み込み
	SoundData soundData1 = xAudio2.SoundLoad("resources/gold.mp3");
	Logger::Write("音声読み込み");
	DebugCamera debugCamera;
	debugCamera.Initialize();

	sprite->Create(tHChecker, positoin, Color::WHITE);
	
	sprite->SetRotation(rotation);
	Vector4 materialColor = sprite->GetColor();

	// ウィンドウの×ボタンが押されるまでループ
	while (app->ProcessMessage()) {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		input.Update();

		/*-- 更新処理 --*/
		if (input.IsPressed(DIK_SPACE)) {
			xAudio2.SoundPlayWave(soundData1);
		}

		debugCamera.Update();

		sprite->SetPosition(positoin);
		sprite->SetColor(materialColor);
		sprite->Update();

		//materialData->color.x = modelColor[0];
		//materialData->color.y = modelColor[1];
		//materialData->color.z = modelColor[2];
		//materialData->color.w = modelColor[3];

		//sprite->SetTransform(transformSprite);
		//sprite->SetMaterial(spriteMaterial);
		//sprite->SetTexture(textureSrvHandleGPU);
		//sprite->SetUvTransform(uvTransformSprite);

		/*Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(app->GetWidth()) / float(app->GetHeight()), 0.1f, 100.0f);
		// WVPMatrixを作る
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->World = worldMatrix;
		wvpData->WVP = worldViewProjectionMatrix;*/

		// 開発用のUIの処理、実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		/*ImGui::SliderAngle("SphereRotateX", &transform.rotate.x);
		ImGui::SliderAngle("SphereRotateY", &transform.rotate.y);
		ImGui::SliderAngle("SphereRotateZ", &transform.rotate.z);
		ImGui::ColorEdit4("modelColor", modelColor);
		ImGui::Checkbox("enableLighting", (bool*)&materialData->enableLighting);*/

		ImGui::ColorEdit4("modelColor", (float*)&materialColor);
		ImGui::SliderFloat2("translateSprite", (float*)&positoin, 0.0f, 1000.0f, "%.3f");
		//ImGui::SliderFloat3("rotateSprite", (float*)&transformSprite.rotate, 0.0f, 10.0f, "%.3f");
		//ImGui::SliderFloat3("scaleSprite", (float*)&transformSprite.scale, 0.0f, 10.0f, "%.3f");
		ImGui::Text("positoin.x : %f", positoin.x);
		ImGui::Text("positoin.y : %f", positoin.y);
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		ShowMemoryUsage();
		//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		// ImGuiの内部コマンドを生成する
		ImGui::Render();


		/*-- 描画処理 --*/
		graphics.BeginFrame();

		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		//cmdList_->SetGraphicsRootSignature(rootSignature.Get());
		/*cmdList_->SetPipelineState(pso3D.Get()); // PSOを設定
		cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
		cmdList_->IASetIndexBuffer(&indexBufferViewModel);

		// 形状を設定。PSOに設定しているものとはまた別。同じものを設定する。
		cmdList_->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

		// wvp用のCBufferの場所を設定
		cmdList_->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

		cmdList_->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
		cmdList_->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

		// 描画 (DrawCall)。
		cmdList_->DrawIndexedInstanced(UINT(modelData.indices.size()), 1, 0, 0, 0);*/

		spriteCommon->DrawCommon();
		sprite->Draw();

		graphics.EndFrame();
	}
	TextureManager::Shutdown();

	input.Shutdown();

	xAudio2.Shutdown();
	xAudio2.SoundUnload(&soundData1);


	graphics.Shutdown();

	Logger::Write("AppのShutdown");
	app->Shutdown();

	Logger::Write("アプリ終了");
	Logger::Shutdown();
	return 0;
}