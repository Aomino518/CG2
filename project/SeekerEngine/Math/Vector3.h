#pragma once

struct Vector3 {
	float x, y, z;

    Vector3 operator+(const Vector3& rhs) const {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    Vector3 operator-(const Vector3& rhs) const {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    Vector3 operator*(const Vector3& rhs) const {
        return { x * rhs.x, y * rhs.y, z * rhs.z };
    }

    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    Vector3 operator/(float scalar) const {
        return { x / scalar, y / scalar, z / scalar };
    }

    Vector3& operator+=(const Vector3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    friend Vector3 operator*(float scalar, const Vector3& v) {
        return { v.x * scalar, v.y * scalar, v.z * scalar };
    }
};