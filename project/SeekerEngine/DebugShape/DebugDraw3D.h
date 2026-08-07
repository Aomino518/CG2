#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DxcCompiler.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "BlendStateUtils.h"
#include "Color.h"
#include <unordered_map>
#include "CreateResorceUtils.h"

class Graphics;
class CameraManager;
class DebugDraw3D
{
public:
	/// <summary>
	/// シングルトン関数
	/// </summary>
	/// <returns>インスタンス</returns>
	static DebugDraw3D* GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Shutdown();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 線を描画する関数
	/// </summary>
	/// <param name="start">始点</param>
	/// <param name="end">終点</param>
	/// <param name="color">色</param>
	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

	/// <summary>
	///	3Dの中心座標基準ワイヤーボックスを描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="halfSize">半分の大きさ</param>
	/// <param name="color">色</param>
	void DrawBoxWire(const Vector3& center, const Vector3& halfSize, const Vector4& color);

	/// <summary>
	///	3Dの中心座標基準ワイヤーボックスを描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="halfSize">半分の大きさ</param>
	/// <param name="color">色</param>
	void DrawBoxSolid(const Vector3& center, const Vector3& halfSize, const Vector4& color);

	/// <summary>
	/// 3Dのワイヤー球を描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	void DrawSphereWire(const Vector3& center, const Vector3& radius, const Vector4& color);

	/// <summary>
	/// 3Dのソリッド球を描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	void DrawSphereSolid(const Vector3& center, const Vector3& radius, const Vector4& color);

	/// <summary>
	/// ワイヤーAABB（軸平行境界ワイヤーボックス）を描画する
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="aabb">描画対象の AABB 軸平行境界ボックス</param>
	/// <param name="color">色</param>
	void DrawAABBWire(const Vector3& position, const AABB& aabb, const Vector4& color);

	/// <summary>
	/// ソリッドAABB（軸平行境界ワイヤーボックス）を描画する
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="aabb">描画対象の AABB 軸平行境界ボックス</param>
	/// <param name="color">色</param>
	void DrawAABBSolid(const Vector3& position, const AABB& aabb, const Vector4& color);

private:
	DebugDraw3D() = default;
	~DebugDraw3D() = default;
	DebugDraw3D(const DebugDraw3D&) = delete;
	DebugDraw3D& operator=(const DebugDraw3D&) = delete;

	void CreateGraphicPipeline(DxcCompiler dxcCompiler);
	void CreateVertexBuffer();
	void CreateTransformationMatrixResource();
	void DrawWire();
	void DrawSolid();

private:
	void DrawPolygon(const Vector3& point1, const Vector3& point2, const Vector3& point3, const Vector4& color);
	Vector4 CreatePoint(const Vector3& center, const Vector3& radius, float lat, float lon);

private:
	// 他クラスファイルからの取得変数
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Graphics* graphics_;

	// シェーダーBlob
	Microsoft::WRL::ComPtr<IDxcBlob> vsDebug3DBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> psDebug3DBlob_;

	// PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoDebug3DWire_; // ワイヤー用
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoDebug3DSolid_; // ソリッド用
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	// ブレンドモード(使うかは未定)
	D3D12_BLEND_DESC blendDesc_{};
	BlendMode mode_ = kBlendModeNone;
	std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// Wire用
	Microsoft::WRL::ComPtr<ID3D12Resource> wireVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW wireVertexBufferView_{};
	DebugVertex* wireVertexData_ = nullptr;
	uint32_t wireVertexCount_ = 0;

	// Solid用
	Microsoft::WRL::ComPtr<ID3D12Resource> solidVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW solidVertexBufferView_{};
	DebugVertex* solidVertexData_ = nullptr;
	uint32_t solidVertexCount_ = 0;

	// 頂点最大数
	static constexpr uint32_t kMaxVertexCount = 8192;

	// TransformationMatrix
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// カメラ
	CameraManager* cameraManager_ = nullptr;
};

