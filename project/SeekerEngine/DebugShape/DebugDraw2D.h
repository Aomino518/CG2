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
class DebugDraw2D
{
public:
	static DebugDraw2D* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxcCompiler">DXCコンパイラ</param>
	/// <param name="rootSignature">ルートシグネチャ</param>
	void Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Shutdown();

	/// <summary>
	/// 線を描画する関数
	/// </summary>
	/// <param name="start">始点</param>
	/// <param name="end">終点</param>
	/// <param name="color">色</param>
	void DrawLine(const Vector2& start, const Vector2& end, const Vector4& color);

	/// <summary>
	/// ワイヤー円を描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radiusX">半径X</param>
	/// <param name="radiusY">半径Y</param>
	/// <param name="color">色</param>
	void DrawCircleWire(const Vector2& center, float radiusX, float radiusY, const Vector4& color);

	/// <summary>
	/// ソリッド円を描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radiusX">半径X</param>
	/// <param name="radiusY">半径Y</param>
	/// <param name="color">色</param>
	void DrawCircleSolid(const Vector2& center, float radiusX, float radiusY, const Vector4& color);

	/// <summary>
	/// 中心座標のワイヤー矩形を描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="halfSize">半サイズ</param>
	/// <param name="color">色</param>
	void DrawBoxWire(const Vector2& center, const Vector2& halfSize, const Vector4& color);

	/// <summary>
	/// 中心座標のソリッド矩形を描画する関数
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="halfSize">半サイズ</param>
	/// <param name="color">色</param>
	void DrawBoxSolid(const Vector2& center, const Vector2& halfSize, const Vector4& color);

	/// <summary>
	/// 左上座標中心のワイヤー矩形を描画する関数
	/// </summary>
	/// <param name="leftTop">左上座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	void DrawRectWire(const Vector2& leftTop, const Vector2& size, const Vector4& color);

	/// <summary>
	/// 左上座標中心のソリッド矩形を描画する関数
	/// </summary>
	/// <param name="leftTop">左上座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	void DrawRectSolid(const Vector2& leftTop, const Vector2& size, const Vector4& color);

	/// <summary>
	/// ワイヤーAABB2D（軸平行境界ワイヤーボックス）を描画する
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="aabb">描画対象の AABB 軸平行境界ボックス</param>
	/// <param name="color">色</param>
	void DrawAABB2DWire(const Vector2& position, const AABB2D& aabb, const Vector4& color);

	/// <summary>
	/// ソリッドAABB2D（軸平行境界ワイヤーボックス）を描画する
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="aabb">描画対象の AABB 軸平行境界ボックス</param>
	/// <param name="color">色</param>
	void DrawAABB2DSolid(const Vector2& position, const AABB2D& aabb, const Vector4& color);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private:
	DebugDraw2D() = default;
	~DebugDraw2D() = default;
	DebugDraw2D(const DebugDraw2D&) = delete;
	DebugDraw2D& operator=(const DebugDraw2D&) = delete;

	void CreateGraphicPipeline(DxcCompiler dxcCompiler);
	void CreateVertexBuffer();
	void CreateTransformationMatrixResource();
	void DrawWire();
	void DrawSolid();
	void DrawPolygon(const Vector2& point1, const Vector2& point2, const Vector2& point3, const Vector4& color);

private:
	// 他クラスファイルからの取得変数
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Graphics* graphics_;

	// シェーダーBlob
	Microsoft::WRL::ComPtr<IDxcBlob> vsDebug2DBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> psDebug2DBlob_;

	// PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoDebug2DWire_; // ワイヤー用
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoDebug2DSolid_; // ソリッド用
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

	// ブレンドモード(使うかは未定)
	D3D12_BLEND_DESC blendDesc_{};
	BlendMode mode_ = kBlendModeNone;
	std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// Wire用
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	DebugVertex* vertexData_ = nullptr;
	uint32_t vertexCount_ = 0;

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
};

