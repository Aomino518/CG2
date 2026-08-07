#pragma once
#include <nlohmann/json.hpp>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

using json = nlohmann::json;

inline json ToJson(const Vector2& v) {
	return json{
		{"x", v.x},
		{"y", v.y}
	};
}

inline json ToJson(const Vector3& v)
{
    return json{
        {"x", v.x},
        {"y", v.y},
        {"z", v.z}
    };
}

inline json ToJson(const Vector4& v)
{
    return json{
        {"x", v.x},
        {"y", v.y},
        {"z", v.z},
        {"w", v.w}
    };
}

inline void FromJson(const json& j, Vector2& v) {
    v.x = j.value("x", v.x);
    v.y = j.value("y", v.y);
}

inline void FromJson(const json& j, Vector3& v) {
    v.x = j.value("x", v.x);
    v.y = j.value("y", v.y);
    v.z = j.value("z", v.z);
}

inline void FromJson(const json& j, Vector4& v) {
    v.x = j.value("x", v.x);
    v.y = j.value("y", v.y);
    v.z = j.value("z", v.z);
    v.w = j.value("w", v.w);
}
