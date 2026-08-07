#pragma once
#include "MathTypes.h"

// =========================
// Matrix4x4
// =========================
Matrix4x4 operator+(const Matrix4x4& lhs, const Matrix4x4& rhs);
Matrix4x4 operator-(const Matrix4x4& lhs, const Matrix4x4& rhs);
Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs);
Matrix4x4 operator*(const Matrix4x4& mat, float scalar);
Matrix4x4 operator*(float scalar, const Matrix4x4& mat);

Matrix4x4& operator+=(Matrix4x4& lhs, const Matrix4x4& rhs);
Matrix4x4& operator-=(Matrix4x4& lhs, const Matrix4x4& rhs);
Matrix4x4& operator*=(Matrix4x4& mat, float scalar);
Matrix4x4& operator*=(Matrix4x4& lhs, const Matrix4x4& rhs);