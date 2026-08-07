#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "BlendStateUtils.h"
#include "CreateResorceUtils.h"

namespace ImGuiUtils
{
	bool DrawTransform2D(Vector2& position, float& rotation, Vector2& scale);
	bool DrawTransform3D(Vector3& position, Vector3& rotation, Vector3& scale);
	bool DrawBlendModeSelector(const char* label, BlendMode& blendMode);
	bool DrawColor4(const char* label, Vector4& color);
	bool DrawVector3(const char* label, Vector3& value);
	bool DrawEditParticleConfig(ParticleConfig& config);
	bool DrawEditParticleConfig2D(ParticleConfig& config);
};

