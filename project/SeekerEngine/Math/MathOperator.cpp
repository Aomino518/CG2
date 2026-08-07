#include "MathOperator.h"
#include "MathFunc.h"

// =========================
// Matrix4x4
// =========================
Matrix4x4 operator+(const Matrix4x4& lhs, const Matrix4x4& rhs) {
    return Add(lhs, rhs);
}

Matrix4x4 operator-(const Matrix4x4& lhs, const Matrix4x4& rhs) {
    return Subtract(lhs, rhs);
}

Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs) {
    return Multiply(lhs, rhs);
}

Matrix4x4 operator*(const Matrix4x4& mat, float scalar) {
    Matrix4x4 result{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = mat.m[i][j] * scalar;
        }
    }
    return result;
}

Matrix4x4 operator*(float scalar, const Matrix4x4& mat) {
    return mat * scalar;
}

Matrix4x4& operator+=(Matrix4x4& lhs, const Matrix4x4& rhs) {
    lhs = lhs + rhs;
    return lhs;
}

Matrix4x4& operator-=(Matrix4x4& lhs, const Matrix4x4& rhs) {
    lhs = lhs - rhs;
    return lhs;
}

Matrix4x4& operator*=(Matrix4x4& lhs, const Matrix4x4& rhs) {
    Matrix4x4 result{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] =
                lhs.m[i][0] * rhs.m[0][j] +
                lhs.m[i][1] * rhs.m[1][j] +
                lhs.m[i][2] * rhs.m[2][j] +
                lhs.m[i][3] * rhs.m[3][j];
        }
    }

    lhs = result;
    return lhs;
}



Matrix4x4& operator*=(Matrix4x4& mat, float scalar) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            mat.m[i][j] *= scalar;
        }
    }
    return mat;
}