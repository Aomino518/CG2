#pragma once

struct Vector4 {
	float x, y, z, w;

    Vector4 operator+(const Vector4& rhs) const {
        return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
    }

    Vector4 operator-(const Vector4& rhs) const {
        return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
    }

    Vector4 operator*(const Vector4& rhs) const {
        return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w };
    }

    Vector4 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar, w * scalar };
    }

    Vector4 operator/(float scalar) const {
        return { x / scalar, y / scalar, z / scalar, w / scalar };
    }

    Vector4& operator+=(const Vector4& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    Vector4& operator-=(const Vector4& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    Vector4& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    Vector4& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    friend Vector4 operator*(float scalar, const Vector4& v) {
        return { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar };
    }
};