#include "Matrix.h"

// 1.行列の加法
Matrix4x4 Add(const Matrix4x4 & m1, const Matrix4x4 & m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[j][i] = m1.m[j][i] + m2.m[j][i];
		}
	}
	return result;
}

// 2.行列の減法
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[j][i] = m1.m[j][i] - m2.m[j][i];
		}
	}
	return result;
}

// 3.行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[j][i] = 0;
			for (int k = 0; k < 4; ++k) {
				result.m[j][i] += m1.m[j][k] * m2.m[k][i];
			}
		}
	}
	return result;
}

// 4.逆行列
Matrix4x4 Inverse(const Matrix4x4& matrix) {
	Matrix4x4 cofactor; // 余因子行列

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			float sub[3][3];
			int subi = 0;
			for (int i = 0; i < 4; ++i) {
				if (i == row) continue;
				int subj = 0;
				for (int j = 0; j < 4; ++j) {
					if (j == col) continue;
					sub[subi][subj] = matrix.m[i][j];
					++subj;
				}
				++subi;
			}

			// 小行列
			float det3 = Determinant3x3(
				sub[0][0], sub[0][1], sub[0][2],
				sub[1][0], sub[1][1], sub[1][2],
				sub[2][0], sub[2][1], sub[2][2]
			);

			cofactor.m[row][col] = ((row + col) % 2 == 0 ? 1 : -1) * det3;
		}
	}

	float det = 0.0f;
	for (int i = 0; i < 4; ++i) {
		det += matrix.m[0][i] * cofactor.m[0][i];
	}

	if (det == 0) {
		Matrix4x4 result = { 0 };
		return result;
	}

	Matrix4x4 result;
	float invDet = 1.0f / det;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = cofactor.m[j][i] * invDet;
		}
	}

	return result;
}

// 5.転置行列
Matrix4x4 Transpose(const Matrix4x4& matrix) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[j][i] = matrix.m[i][j];
		}
	}
	return result;
}

// 6.単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[j][i] = 0.0f;
		}
	}

	for (int i = 0; i < 4; i++) {
		result.m[i][i] = 1.0f;
	}

	return result;
}

float Determinant3x3(
	float a11, float a12, float a13,
	float a21, float a22, float a23,
	float a31, float a32, float a33) {
	return
		a11 * (a22 * a33 - a23 * a32) -
		a12 * (a21 * a33 - a23 * a31) +
		a13 * (a21 * a32 - a22 * a31);
}
