#pragma once
#include "MathFunc.h"
#include "Color.h"
#include "CreateResorceUtils.h"

class DebugDraw
{
public:
	/// <summary>
	/// 描画処理
	/// </summary>
	static void Draw();

	/// <summary>
	/// 更新処理
	/// </summary>
	static void Update();

	/// <summary>
	/// 2Dの線を描画する関数
	/// </summary>
	/// <param name="start">始点</param>
	/// <param name="end">終点</param>
	/// <param name="color">色</param>
	static void DrawLine(const Vector2& start, const Vector2& end, const Vector4& color);

	/// <summary>
	/// 3Dの線を描画する関数
	/// </summary>
	/// <param name="start">始点</param>
	/// <param name="end">終点</param>
	/// <param name="color">色</param>
	static void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

	/// <summary>
	/// 2Dの円を描画する関数
	/// </summary>
	/// <param name="center">中心点</param>
	/// <param name="radiusX">半径X</param>
	/// <param name="radiusY">半径Y</param>
	/// <param name="color">色</param>
	/// <param name="mode">描画モード</param>
	static void DrawCircle(const Vector2& center, float radiusX, float radiusY, const Vector4& color, DebugDrawMode mode);

	/// <summary>
	/// 3Dの球を描画する関数
	/// </summary>
	/// <param name="center">中心点</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	/// <param name="mode">描画モード</param>
	static void DrawSphere(const Vector3& center, const Vector3& radius, const Vector4& color, DebugDrawMode mode);

	/// <summary>
	/// 2Dの中心座標基準の矩形を描画する関数
	/// </summary>
	/// <param name="center">中心点</param>
	/// <param name="halfSize">半分の大きさ</param>
	/// <param name="color">色</param>
	/// <param name="mode">描画モード</param>
	static void DrawBox(const Vector2& center, const Vector2& halfSize, const Vector4& color, DebugDrawMode mode);

	/// <summary>
	/// 3Dのボックスを描画する関数
	/// </summary>
	/// <param name="center">中心点</param>
	/// <param name="size">大きさ</param>
	/// <param name="mode">描画モード</param>
	static void DrawBox(const Vector3& center, const Vector3& halfSize, const Vector4& color, DebugDrawMode mode);

	/// <summary>
	/// 2Dの左上中心座標基準の矩形を描画する関数
	/// </summary>
	/// <param name="leftTop">左上座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	/// <param name="mode">描画モード</param>
	static void DrawRect(const Vector2& leftTop, const Vector2& size, const Vector4& color, DebugDrawMode mode);

	/// <summary>
	/// AABB（軸平行境界ボックス）を描画する
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="aabb">描画対象の AABB 軸平行境界ボックス</param>
	/// <param name="mode">描画モード</param>
	static void DrawAABB(const Vector3& position, const AABB& aabb, const Vector4& color, DebugDrawMode mode);

	// <summary>
	/// AABB（軸平行境界ボックス）を描画する
	/// </summary>
	/// <param name="position">座標</param>
	/// <param name="aabb">描画対象の AABB2D 軸平行境界ボックス</param>
	/// <param name="mode">描画モード</param>
	static void DrawAABB2D(const Vector2& position, const AABB2D& aabb, const Vector4& color, DebugDrawMode mode);
};

