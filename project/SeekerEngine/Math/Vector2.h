#pragma once

struct Vector2 {
	float x, y;

    Vector2 operator+(const Vector2& rhs) const {
        return { x + rhs.x, y + rhs.y };
    }

    Vector2 operator-(const Vector2& rhs) const {
        return { x - rhs.x, y - rhs.y };
    }

    Vector2 operator*(const Vector2& rhs) const {
        return { x * rhs.x, y * rhs.y };
    }

    Vector2 operator*(float scalar) const {
        return { x * scalar, y * scalar };
    }

    Vector2 operator/(float scalar) const {
        return { x / scalar, y / scalar };
    }

    Vector2& operator+=(const Vector2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    friend Vector2 operator*(float scalar, const Vector2& v) {
        return { v.x * scalar, v.y * scalar };
    }
};