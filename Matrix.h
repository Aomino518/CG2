#pragma once

struct Matrix4x4 {
	float m[4][4];
};

// 1.行列の加法
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
// 2.行列の減法
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
// 3.行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
// 4.逆行列
Matrix4x4 Inverse(const Matrix4x4& matrix);
// 5.転置行列
Matrix4x4 Transpose(const Matrix4x4& matrix);
// 6.単位行列の作成
Matrix4x4 MakeIdentity4x4();

float Determinant3x3(
	float a11, float a12, float a13,
	float a21, float a22, float a23,
	float a31, float a32, float a33);
