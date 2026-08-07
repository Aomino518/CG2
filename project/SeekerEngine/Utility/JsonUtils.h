#pragma once
#include <string>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

namespace JsonUtils
{
	std::string ReadFilePath(const std::string& path);
	void WriteTextFile(const std::string& path, const std::string& text);
};

