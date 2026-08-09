#pragma once
#include <cstdint>
#include "CreateResorceUtils.h"
#include "MathOperator.h"
#include "AABB.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct Sphere {
	Vector3 center;
	float radius;
};

// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2);
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

// 減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

Vector3 Multiply(const float& scalor, const Vector3& v);
// <summary>
/// X軸回転行列関数
/// </summary>
/// <param name="radian">ラジアン</param>
/// <returns>X軸回転行列</returns>
Matrix4x4 MakeRotateXMatrix(float radian);
/// <summary>
/// Y軸回転行列関数
/// </summary>
/// <param name="radian">ラジアン</param>
/// <returns>Y軸回転行列</returns>
Matrix4x4 MakeRotateYMatrix(float radian);
/// <summary>
/// Z軸回転行列関数
/// </summary>
/// <param name="radian">ラジアン</param>
/// <returns>Z軸回転行列</returns>
Matrix4x4 MakeRotateZMatrix(float radian);
/// <summary>
/// 行列の積を求める関数
/// </summary>
/// <param name="m1">行列1</param>
/// <param name="m2">行列2</param>
/// <returns>行列の積</returns>
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
/// <summary>
/// 平行移動行列関数
/// </summary>
/// <param name="translate">方向</param>
/// <returns>平行移動行列</returns>
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
/// <summary>
/// 拡大縮小行列関数
/// </summary>
/// <param name="scale">スケール</param>
/// <returns>拡大縮小行列</returns>
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
/// <summary>
/// 透視投影行列の作成関数
/// </summary>
/// <param name="fovY">画角Y</param>
/// <param name="aspectRatio">アスペクト比</param>
/// <param name="nearClip">近平面への距離</param>
/// <param name="farClip">遠平面への距離</param>
/// <returns>透視投影行列</returns>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
/// <summary>
/// 3次元アフィン変換行列
/// </summary>
/// <param name="scale">拡縮行列</param>
/// <param name="rotate">回転行列</param>
/// <param name="translate">平行移動行列</param>
/// <returns></returns>
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
/// <summary>
/// 逆行列
/// </summary>
/// <param name="matrix"></param>
/// <returns></returns>
Matrix4x4 Inverse(const Matrix4x4& matrix);

// 単位行列の作成
Matrix4x4 MakeIdentity4x4();

float Determinant3x3(
	float a11, float a12, float a13,
	float a21, float a22, float a23,
	float a31, float a32, float a33);

/// <summary>
/// 正射影行列の作成関数
/// </summary>
/// <param name="left">左上のX座標</param>
/// <param name="top">左上のY座標</param>
/// <param name="right">右下のX座標</param>
/// <param name="bottom">右下のY座標</param>
/// <returns>正射影行列</returns>
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

float Length(const Vector3& vector);
Vector3 Normalize(const Vector3& normal);
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);
Matrix4x4 Transpose(const Matrix4x4& matrix);

bool IsCollision(const AABB& aabb, const Vector3& point);
bool IsCollision(const AABB2D& aabb, const Vector3& point);

// 球同士の当たり判定を返す関数
bool IsCollision(const Sphere& s1, const Sphere& s2);

/// <summary>
/// 線形補完関数
/// </summary>
/// <param name="start">始まりの数値</param>
/// <param name="end">終わりの数値</param>
/// <param name="t">時間</param>
/// <returns>線形補完値</returns>
float Lerp(const float start, const float end, float t);

Vector3 GetForwardFromTransform(const Vector3& rotate);

// 座標変換
Vector3 TransformToVector3(const Vector3& vector, const Matrix4x4& matrix);

// オペレーター
Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 operator*(const Matrix4x4& mat, float scalar);

Matrix4x4 operator*(float scalar, const Matrix4x4& mat);

Matrix4x4& operator*=(Matrix4x4& mat, float scalar);

Matrix4x4& operator*=(Matrix4x4& lhs, const Matrix4x4& rhs);

// 線形補間関数Vector3
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
// 線形補間関数Vector4
Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t);