#pragma once
#include <nlohmann/json.hpp>
#include "JsonMath.h"
#include "CreateResorceUtils.h"

inline json TransformToJson(const Transform& t) {
	return json{
		{"translate", ToJson(t.translate)},
		{"rotate", ToJson(t.rotate)},
		{"scale", ToJson(t.scale)}
	};
}

inline void TransformFromJson(const json& j, Transform& t) {
	FromJson(j.at("translate"), t.translate);
	FromJson(j.at("rotate"), t.rotate);
	FromJson(j.at("scale"), t.scale);
}