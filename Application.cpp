#include "Application.h"
#include <format>
#include <dxgidebug.h>
#include <dbghelp.h>
#include <strsafe.h>
#include <sstream>
#include <dxcapi.h>
#include <vector>
#include "Matrix.h"
#include "DebugCamera.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#define _USE_MATH_DEFINES 
#include <math.h>
#include <dinput.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxcompiler.lib")


#include "Logger.h"
#include "StringUtil.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma region 構造体
struct Material {
	Vector4 color;
	bool enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {
		// リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

#pragma endregion

#pragma region 自作関数
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

DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	// テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミップマップ付きのデータを返す
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata) {
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数、普段使ってるのは2次元

	// 利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // プロセッサの近くに配置

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr;
	hr = device->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST, // 初回のResourceState。Textureは基本読むだけ
		nullptr, // Clear最適値。使わないのでnullpr
		IID_PPV_ARGS(&resource) // 作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));
	return resource;
}

[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
	Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
	const DirectX::ScratchImage& mipImages,
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediasteResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture.Get(), intermediasteResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	// Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_RTEADへResourceStateへ変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediasteResource;
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
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				position.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;
				normal.x *= -1.0f;
				//VertexData vertex = { position, texcoord, normal };
				//modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position, texcoord, normal };
			}
			// 頂点を逆順で登録、回り順を逆順にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);

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

#pragma endregion

Application::Application(int width, int height, const wchar_t* title)
	: hInstance_(GetModuleHandle(nullptr)), title_(title ? title : L"CG2"),
		clientWidth_(width), clientHeight_(height) {}

Application::~Application()
{
	Shutdown();
}

bool Application::Init()
{
	/*--ウィンドウクラスの登録--*/
	WNDCLASS wc{};

	// ウィンドウプロシージャ
	wc.lpfnWndProc = Application::WndProcSetup;

	// ウィンドウクラス名
	wc.lpszClassName = className_.c_str();

	// インスタンスハンドル
	wc.hInstance = hInstance_;

	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);

	/*--ウィンドウサイズを決定--*/
	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, clientWidth_, clientHeight_ };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, style_, false);

	/*--ウィンドウを生成する--*/
	hwnd_ = CreateWindow(
		className_.c_str(), // 利用するクラス名
		title_.c_str(), // タイトルバーの文字
		style_, // よく見るウィンドウスタイル
		CW_USEDEFAULT, // 表示X座標(Windowsに任せる)
		CW_USEDEFAULT, // 表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left, // ウィンドウ横幅
		wrc.bottom - wrc.top, // ウィンドウ縦幅
		nullptr, // 親ウィンドウハンドル
		nullptr, // メニューハンドル
		hInstance_, // インスタンスハンドル
		this // オプション
	);

	if (!hwnd_) {
		return false;
	}

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);
	isRunning_ = true;
	return true;
}

bool Application::ProcessMessage()
{
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) { 
			isRunning_ = false; return false; 
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return isRunning_;
}

void Application::Run()
{
	D3DResourceLeakChecker leakCheck;
	CoInitializeEx(0, COINIT_MULTITHREADED);
	SetUnhandledExceptionFilter(ExportDump);

	// graphicsの初期化
	graphics_.Init(hwnd_, clientWidth_, clientHeight_, true);

    // XAudio2の初期化
	xAudio2_.Init();

	//DirectInput初期化
	input_.Init(hInstance_, hwnd_);

	// デバイスの生成がうまくいかなかったので起動できない
	assert(graphics_.GetDevice() != nullptr);
	// 初期化完了ログ
	Logger::Write("Complete Create D3D12Device!!!");

	ID3D12GraphicsCommandList* cmdList_ = graphics_.GetCmdList();

	// DxcCompilerの初期化
	dxcCompiler_.Init();

	// RootSignature作成
	rootSignatureFactory_.Init(&graphics_);
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = rootSignatureFactory_.CreateCommon();

#pragma region Texture読み込み
	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(graphics_.GetDevice(), metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediasteResource = UploadTextureData(textureResource, mipImages, graphics_.GetDevice(), cmdList_);

	// モデル読み込み
	ModelData modelData = LoadObjFile("resources", "axis.obj");
	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(graphics_.GetDevice(), metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediasteResource2 = UploadTextureData(textureResource2, mipImages2, graphics_.GetDevice(), cmdList_);
#pragma endregion

#pragma region SRVの設定
	// metadataを基にSRVを設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap = graphics_.GetSRVHeap();
	uint32_t descSizeSRV = graphics_.GetSRVDescriptorSize();

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvHeap, descSizeSRV, 0);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvHeap, descSizeSRV, 0);
	// 先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += graphics_.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += graphics_.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRVの生成
	graphics_.GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	// 2つ目のSRVの作成
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvHeap, descSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvHeap, descSizeSRV, 2);
	// SRVの生成
	graphics_.GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
#pragma endregion

#pragma region リソース設定
	/*--モデル用のリソース設定--*/
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(graphics_.GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = Vector4(1, 1, 1, 1);
	materialData->uvTransform = MakeIdentity4x4();
	materialData->enableLighting = true;

	/*--スプライト用のリソース設定--*/
	// Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(graphics_.GetDevice(), sizeof(Material));
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// SpriteはLightingしないのでfalseを設定する
	materialDataSprite->color = Vector4(1, 1, 1, 1);
	materialDataSprite->uvTransform = MakeIdentity4x4();
	materialDataSprite->enableLighting = false;

	// 平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(graphics_.GetDevice(), sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 初期化値
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 1.0f, 0.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(graphics_.GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();

	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(graphics_.GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// 単位行列を書きこんでおく
	transformationMatrixDataSprite->World = MakeIdentity4x4();
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();

	//const uint32_t kSubdivision = 16; // 16分割

	//uint32_t vertexNum = (kSubdivision + 1) * (kSubdivision + 1);
	//uint32_t indexNum = kSubdivision * kSubdivision * 6;

	/*--Index用リソース作成--*/
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(graphics_.GetDevice(), sizeof(uint32_t) * 6);
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceModel = CreateBufferResource(graphics_.GetDevice(), sizeof(uint32_t) * 6);
#pragma endregion

	// InputLayout
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc3D{};
	inputLayoutDesc3D = inputLayout_.CreateInputLayout3D();

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//blendDesc.RenderTarget[0].BlendEnable = TRUE;
	//blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	//blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	//blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	dxcCompiler_.ShaderBlob(L"Object3D.VS.hlsl", L"vs_6_0", L"Object3D.PS.hlsl", L"ps_6_0");

	IDxcBlob* vertexShaderBlob = dxcCompiler_.GetVertexShaderBlob();
	IDxcBlob* pixelShaderBlob = dxcCompiler_.GetPixelShaderBlob();

	// PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc3D;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = graphics_.GetDepthStencilDesc();
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	HRESULT hr = graphics_.GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// 頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(graphics_.GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	/*--IndexBufferViewの作成--*/
	/*--Sprite用--*/
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	// Sprite用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(graphics_.GetDevice(), sizeof(VertexData) * 4);
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	/*--Model用--*/
	D3D12_INDEX_BUFFER_VIEW indexBufferViewModel{};
	// リソースの先頭のアドレスから使う
	indexBufferViewModel.BufferLocation = indexResourceModel->GetGPUVirtualAddress();
	indexBufferViewModel.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferViewModel.Format = DXGI_FORMAT_R32_UINT;


	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	// 矩形
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[2].position = { 360.0f, 360.0f, 0.0f, 1.0f };
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[3].position = { 360.0f, 0.0f, 0.0f, 1.0f };
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };

	// インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	//uint32_t index = 0;
	uint32_t* indexDataModel = nullptr;
	indexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&indexDataModel));
	std::memcpy(indexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

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

	// ビューポート
	D3D12_VIEWPORT viewport{};
	// クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = float(GetWidth());
	viewport.Height = float(GetHeight());
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// シザー矩形
	D3D12_RECT scissorRect{};
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = GetWidth();
	scissorRect.top = 0;
	scissorRect.bottom = GetHeight();

	// Transform変数を作る
	Transform transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	//Transform cameraTransform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };
	Transform transformSprite = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	Transform uvTransformSprite = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = graphics_.GetSwapChainDesc();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = graphics_.GetRTVDesc();

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(GetHWND());
	ImGui_ImplDX12_Init(graphics_.GetDevice(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvHeap.Get(),
		srvHeap->GetCPUDescriptorHandleForHeapStart(),
		srvHeap->GetGPUDescriptorHandleForHeapStart());

	// 初期色 (RGBA)
	float modelColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float triangleColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float translateSprite[3] = { 0.0f, 0.0f, 0.0f };

	float transformRotate[3] = { 0.0f, 0.0f, 0.0f };

	// Textureの切り替え変数
	bool useMonsterBall = false;

	// 音声読み込み
	SoundData soundData1 = xAudio2_.SoundLoad("resources/gold.mp3");
	DebugCamera debugCamera;
	debugCamera.Initialize();

	const float clear[4] = { 0.1f, 0.25f, 0.5f, 1.0f };

	// ウィンドウの×ボタンが押されるまでループ
	while (ProcessMessage()) {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		input_.Update();

		/*-- 更新処理 --*/
 		if (input_.IsPressed(DIK_SPACE)) {
			xAudio2_.SoundPlayWave(soundData1);
		}

		debugCamera.Update();

		transformSprite.translate.x = translateSprite[0];
		transformSprite.translate.y = translateSprite[1];
		transformSprite.translate.z = translateSprite[2];

		materialData->color.x = modelColor[0];
		materialData->color.y = modelColor[1];
		materialData->color.z = modelColor[2];
		materialData->color.w = modelColor[3];

		materialDataSprite->color.x = triangleColor[0];
		materialDataSprite->color.y = triangleColor[1];
		materialDataSprite->color.z = triangleColor[2];
		materialDataSprite->color.w = triangleColor[3];

		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;

		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(GetWidth()) / float(GetHeight()), 0.1f, 100.0f);
		// WVPMatrixを作る
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->World = worldMatrix;
		wvpData->WVP = worldViewProjectionMatrix;

		// Sprite用のWorldViewProjectionMatrixを作る
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(GetWidth()), float(GetHeight()), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		transformationMatrixDataSprite->World = worldViewProjectionMatrixSprite;
		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

		// 開発用のUIの処理、実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		ImGui::SliderAngle("SphereRotateX", &transform.rotate.x);
		ImGui::SliderAngle("SphereRotateY", &transform.rotate.y);
		ImGui::SliderAngle("SphereRotateZ", &transform.rotate.z);
		ImGui::ColorEdit4("modelColor", modelColor);
		ImGui::Checkbox("enableLighting", &materialData->enableLighting);

		ImGui::ColorEdit4("spriteColor", triangleColor);
		ImGui::SliderFloat3("translateSprite", (float*)&translateSprite, 0.0f, 1000.0f, "%.3f");
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		// ImGuiの内部コマンドを生成する
		ImGui::Render();


		/*-- 描画処理 --*/
		graphics_.BeginFrame(clear);
		{
			// 描画用のDescriptorHeapの設定
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvHeap.Get() };
			cmdList_->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

			cmdList_->RSSetViewports(1, &viewport); // Viewportを設定
			cmdList_->RSSetScissorRects(1, &scissorRect); // Scissorを設定
			// RootSignatureを設定。PSOに設定しているけど別途設定が必要
			cmdList_->SetGraphicsRootSignature(rootSignature.Get());
			cmdList_->SetPipelineState(graphicsPipelineState.Get()); // PSOを設定
			cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
			cmdList_->IASetIndexBuffer(&indexBufferViewModel);
			// 形状を設定。PSOに設定しているものとはまた別。同じものを設定する。
			cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			cmdList_->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

			// wvp用のCBufferの場所を設定
			cmdList_->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

			cmdList_->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

			// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
			cmdList_->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

			// 描画 (DrawCall)。
			cmdList_->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

			// Spriteの描画。変更が必要なものだけを変更する
			cmdList_->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
			cmdList_->IASetIndexBuffer(&indexBufferViewSprite);// IBV設定

			// スプライト用マテリアルのCBufferの場所を設定
			cmdList_->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());

			cmdList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

			// 描画 (DrawCall)。3頂点で一つのインスタンス
			cmdList_->DrawIndexedInstanced(6, 1, 0, 0, 0);


			// 実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList_);
		}
		graphics_.EndFrame();
	}

	input_.Shutdown();

	xAudio2_.Shutdown();
	xAudio2_.SoundUnload(&soundData1);

	// ImGuiの終了処理。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	graphics_.Shutdown();
}

void Application::Shutdown()
{
	if (hwnd_) {
		DestroyWindow(hwnd_);
		hwnd_ = nullptr;
	}
	UnregisterClass(className_.c_str(), hInstance_);
	isRunning_ = false;
}

LRESULT Application::WndProcSetup(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == WM_NCCREATE) {
		auto cs = reinterpret_cast<CREATESTRUCT*>(lp);
		auto that = reinterpret_cast<Application*>(cs->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Application::WndProcThunk));
		return that->WndProc(hWnd, msg, wp, lp);
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT Application::WndProcThunk(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	auto that = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return that ? that->WndProc(hWnd, msg, wp, lp) : DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT Application::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) {
		return true;
	}

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
	case WM_CLOSE:
		// ここで明示的に破棄
		DestroyWindow(hWnd);
		return 0;
	case WM_SIZE:
		clientWidth_ = LOWORD(lp);
		clientHeight_ = HIWORD(lp);
		return 0;
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理行う
	return DefWindowProc(hWnd, msg, wp, lp);
}


