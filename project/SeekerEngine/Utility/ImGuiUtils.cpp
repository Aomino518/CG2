#define NOMINMAX
#include "ImGuiUtils.h"
#include <algorithm>
#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#endif

bool ImGuiUtils::DrawTransform2D(Vector2& position, float& rotation, Vector2& scale)
{
#ifdef USE_IMGUI
	bool changed = false;

	if (ImGui::TreeNode("Transform")) {
		changed |= ImGui::DragFloat2("Position", reinterpret_cast<float*>(&position), 0.01f);
		changed |= ImGui::DragFloat("Rotation", &rotation, 0.1f);
		changed |= ImGui::DragFloat2("Scale", reinterpret_cast<float*>(&scale), 0.01f, 0.0f);
		ImGui::TreePop();
	}

	return changed;
#endif
}

bool ImGuiUtils::DrawTransform3D(Vector3& position, Vector3& rotation, Vector3& scale)
{
#ifdef USE_IMGUI
	bool changed = false;

	if (ImGui::TreeNode("Transform")) {
		changed |= ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position), 0.01f);
		changed |= ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rotation), 0.01f);
		changed |= ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&scale), 0.01f, 0.0f);
		ImGui::TreePop();
	}

	return changed;
#endif
}

bool ImGuiUtils::DrawBlendModeSelector(const char* label, BlendMode& blendMode)
{
#ifdef USE_IMGUI
	static const char* blendNames[] = {
		"None",
		"Normal",
		"Add",
		"Subtract",
		"Multiply",
		"Screen"
	};

	int current = static_cast<int>(blendMode);

	if (ImGui::Combo(label, &current, blendNames, IM_ARRAYSIZE(blendNames))) {
		blendMode = static_cast<BlendMode>(current);
		return true;
	}

	return false;
#endif
}

bool ImGuiUtils::DrawColor4(const char* label, Vector4& color)
{
#ifdef USE_IMGUI
	bool changed = false;

	if (ImGui::TreeNode(label))
	{
		changed |= ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color));
		ImGui::TreePop();
	}

	return changed;
#endif
}

bool ImGuiUtils::DrawVector3(const char* label, Vector3& value)
{
#ifdef USE_IMGUI
	bool changed = false;

	changed |= ImGui::DragFloat3(label, reinterpret_cast<float*>(&value), 0.01f);

	return changed;
#endif
}

bool ImGuiUtils::DrawEditParticleConfig(ParticleConfig& config)
{
#ifdef USE_IMGUI

	bool changed = false;

	// =========================
	// Spawn Shape
	// =========================
	if (ImGui::CollapsingHeader("Spawn Shape", ImGuiTreeNodeFlags_DefaultOpen)) {

		int shape = static_cast<int>(config.shape);
		const char* shapes[] = { "Box", "Sphere" };
		bool shapeChanged = ImGui::Combo("Shape", &shape, shapes, IM_ARRAYSIZE(shapes));
		changed |= shapeChanged;

		if (shapeChanged) {
			config.shape = static_cast<SpawnShape>(shape);
		}

		if (config.shape == SpawnShape::Box) {
			changed |= ImGui::DragFloat3("Box Min", &config.boxMin.x, 0.1f);
			changed |= ImGui::DragFloat3("Box Max", &config.boxMax.x, 0.1f);
		} else if (config.shape == SpawnShape::Sphere) {
			changed |= ImGui::DragFloat("Radius", &config.sphereRadius, 0.01f, 0.01f, 100.0f);
		}
	}

	// =========================
	// Velocity / Rotation
	// =========================
	if (ImGui::CollapsingHeader("Velocity / Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::DragFloat3("Min Velocity", &config.minVelocity.x, 0.1f);
		changed |= ImGui::DragFloat3("Max Velocity", &config.maxVelocity.x, 0.1f);

		changed |= ImGui::DragFloat3("Min Rotate", &config.minRotate.x, 0.1f);
		changed |= ImGui::DragFloat3("Max Rotate", &config.maxRotate.x, 0.1f);

		changed |= ImGui::DragFloat3("Min RotVel", &config.minRotateVelocity.x, 0.1f);
		changed |= ImGui::DragFloat3("Max RotVel", &config.maxRotateVelocity.x, 0.1f);
	}

	// =========================
	// Color
	// =========================
	if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::ColorEdit4("Start Color Min", &config.startColorMin.x);
		changed |= ImGui::ColorEdit4("Start Color Max", &config.startColorMax.x);
		changed |= ImGui::ColorEdit4("End Color Min", &config.endColorMin.x);
		changed |= ImGui::ColorEdit4("End Color Max", &config.endColorMax.x);
	}

	// =========================
	// Scale
	// =========================
	if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::DragFloat3("Start Scale Min", &config.startScaleMin.x, 0.1f);
		changed |= ImGui::DragFloat3("Start Scale Max", &config.startScaleMax.x, 0.1f);
		changed |= ImGui::DragFloat3("End Scale Min", &config.endScaleMin.x, 0.1f);
		changed |= ImGui::DragFloat3("End Scale Max", &config.endScaleMax.x, 0.1f);
	}

	// =========================
	// Lifetime
	// =========================
	if (ImGui::CollapsingHeader("Lifetime", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::DragFloat("Min Life", &config.minLifeTime, 0.1f, 0.01f);
		changed |= ImGui::DragFloat("Max Life", &config.maxLifeTime, 0.1f, 0.01f);
	}

	config.minOffset.x = std::min(config.minOffset.x, config.maxOffset.x);
	config.minOffset.y = std::min(config.minOffset.y, config.maxOffset.y);
	config.minOffset.z = std::min(config.minOffset.z, config.maxOffset.z);

	config.maxOffset.x = std::max(config.minOffset.x, config.maxOffset.x);
	config.maxOffset.y = std::max(config.minOffset.y, config.maxOffset.y);
	config.maxOffset.z = std::max(config.minOffset.z, config.maxOffset.z);

	config.minLifeTime = std::max(0.01f, config.minLifeTime);
	config.maxLifeTime = std::max(config.minLifeTime, config.maxLifeTime);

	config.startScaleMin.x = std::max(0.0f, config.startScaleMin.x);
	config.startScaleMin.y = std::max(0.0f, config.startScaleMin.y);
	config.startScaleMin.z = std::max(0.0f, config.startScaleMin.z);

	config.startScaleMax.x = std::max(config.startScaleMin.x, config.startScaleMax.x);
	config.startScaleMax.y = std::max(config.startScaleMin.y, config.startScaleMax.y);
	config.startScaleMax.z = std::max(config.startScaleMin.z, config.startScaleMax.z);

	config.endScaleMin.x = std::max(0.0f, config.endScaleMin.x);
	config.endScaleMin.y = std::max(0.0f, config.endScaleMin.y);
	config.endScaleMin.z = std::max(0.0f, config.endScaleMin.z);

	config.endScaleMax.x = std::max(config.endScaleMin.x, config.endScaleMax.x);
	config.endScaleMax.y = std::max(config.endScaleMin.y, config.endScaleMax.y);
	config.endScaleMax.z = std::max(config.endScaleMin.z, config.endScaleMax.z);

	config.maxVelocity.x = std::max(config.minVelocity.x, config.maxVelocity.x);
	config.maxVelocity.y = std::max(config.minVelocity.y, config.maxVelocity.y);
	config.maxVelocity.z = std::max(config.minVelocity.z, config.maxVelocity.z);

	config.maxRotate.x = std::max(config.minRotate.x, config.maxRotate.x);
	config.maxRotate.y = std::max(config.minRotate.y, config.maxRotate.y);
	config.maxRotate.z = std::max(config.minRotate.z, config.maxRotate.z);

	config.maxRotateVelocity.x = std::max(config.minRotateVelocity.x, config.maxRotateVelocity.x);
	config.maxRotateVelocity.y = std::max(config.minRotateVelocity.y, config.maxRotateVelocity.y);
	config.maxRotateVelocity.z = std::max(config.minRotateVelocity.z, config.maxRotateVelocity.z);

	config.startColorMin.x = std::min(config.startColorMin.x, config.startColorMax.x);
	config.startColorMin.y = std::min(config.startColorMin.y, config.startColorMax.y);
	config.startColorMin.z = std::min(config.startColorMin.z, config.startColorMax.z);
	config.startColorMin.w = std::min(config.startColorMin.w, config.startColorMax.w);

	config.startColorMax.x = std::max(config.startColorMin.x, config.startColorMax.x);
	config.startColorMax.y = std::max(config.startColorMin.y, config.startColorMax.y);
	config.startColorMax.z = std::max(config.startColorMin.z, config.startColorMax.z);
	config.startColorMax.w = std::max(config.startColorMin.w, config.startColorMax.w);

	config.endColorMin.x = std::min(config.endColorMin.x, config.endColorMax.x);
	config.endColorMin.y = std::min(config.endColorMin.y, config.endColorMax.y);
	config.endColorMin.z = std::min(config.endColorMin.z, config.endColorMax.z);
	config.endColorMin.w = std::min(config.endColorMin.w, config.endColorMax.w);

	config.endColorMax.x = std::max(config.endColorMin.x, config.endColorMax.x);
	config.endColorMax.y = std::max(config.endColorMin.y, config.endColorMax.y);
	config.endColorMax.z = std::max(config.endColorMin.z, config.endColorMax.z);
	config.endColorMax.w = std::max(config.endColorMin.w, config.endColorMax.w);

	config.boxMin.x = std::min(config.boxMin.x, config.boxMax.x);
	config.boxMin.y = std::min(config.boxMin.y, config.boxMax.y);
	config.boxMin.z = std::min(config.boxMin.z, config.boxMax.z);

	config.boxMax.x = std::max(config.boxMin.x, config.boxMax.x);
	config.boxMax.y = std::max(config.boxMin.y, config.boxMax.y);
	config.boxMax.z = std::max(config.boxMin.z, config.boxMax.z);

	return changed;
#endif
}

bool ImGuiUtils::DrawEditParticleConfig2D(ParticleConfig& config)
{
#ifdef USE_IMGUI

	bool changed = false;

	// =========================
	// Spawn Shape
	// =========================
	if (ImGui::CollapsingHeader("Spawn Shape", ImGuiTreeNodeFlags_DefaultOpen)) {

		int shape = static_cast<int>(config.shape);
		const char* shapes[] = { "Box", "Sphere" };
		bool shapeChanged = ImGui::Combo("Shape", &shape, shapes, IM_ARRAYSIZE(shapes));
		changed |= shapeChanged;

		if (shapeChanged) {
			config.shape = static_cast<SpawnShape>(shape);
		}

		if (config.shape == SpawnShape::Box) {
			changed |= ImGui::DragFloat2("Box Min", &config.boxMin.x, 0.1f);
			changed |= ImGui::DragFloat2("Box Max", &config.boxMax.x, 0.1f);
		} else if (config.shape == SpawnShape::Sphere) {
			changed |= ImGui::DragFloat("Radius", &config.sphereRadius, 0.01f, 0.01f, 100.0f);
		}
	}

	// =========================
	// Velocity / Rotation
	// =========================
	if (ImGui::CollapsingHeader("Velocity / Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::DragFloat2("Min Velocity", &config.minVelocity.x, 0.1f);
		changed |= ImGui::DragFloat2("Max Velocity", &config.maxVelocity.x, 0.1f);

		changed |= ImGui::DragFloat2("Min Rotate", &config.minRotate.x, 0.1f);
		changed |= ImGui::DragFloat2("Max Rotate", &config.maxRotate.x, 0.1f);

		changed |= ImGui::DragFloat2("Min RotVel", &config.minRotateVelocity.x, 0.1f);
		changed |= ImGui::DragFloat2("Max RotVel", &config.maxRotateVelocity.x, 0.1f);
	}

	// =========================
	// Color
	// =========================
	if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::ColorEdit4("Start Color Min", &config.startColorMin.x);
		changed |= ImGui::ColorEdit4("Start Color Max", &config.startColorMax.x);
		changed |= ImGui::ColorEdit4("End Color Min", &config.endColorMin.x);
		changed |= ImGui::ColorEdit4("End Color Max", &config.endColorMax.x);
	}

	// =========================
	// Scale
	// =========================
	if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::DragFloat2("Start Scale Min", &config.startScaleMin.x, 0.1f);
		changed |= ImGui::DragFloat2("Start Scale Max", &config.startScaleMax.x, 0.1f);
		changed |= ImGui::DragFloat2("End Scale Min", &config.endScaleMin.x, 0.1f);
		changed |= ImGui::DragFloat2("End Scale Max", &config.endScaleMax.x, 0.1f);
	}

	// =========================
	// Lifetime
	// =========================
	if (ImGui::CollapsingHeader("Lifetime", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGui::DragFloat("Min Life", &config.minLifeTime, 0.1f, 0.01f);
		changed |= ImGui::DragFloat("Max Life", &config.maxLifeTime, 0.1f, 0.01f);
	}

	config.minOffset.x = std::min(config.minOffset.x, config.maxOffset.x);
	config.minOffset.y = std::min(config.minOffset.y, config.maxOffset.y);
	config.minOffset.z = std::min(config.minOffset.z, config.maxOffset.z);

	config.maxOffset.x = std::max(config.minOffset.x, config.maxOffset.x);
	config.maxOffset.y = std::max(config.minOffset.y, config.maxOffset.y);
	config.maxOffset.z = std::max(config.minOffset.z, config.maxOffset.z);

	config.minLifeTime = std::max(0.01f, config.minLifeTime);
	config.maxLifeTime = std::max(config.minLifeTime, config.maxLifeTime);

	config.startScaleMin.x = std::max(0.0f, config.startScaleMin.x);
	config.startScaleMin.y = std::max(0.0f, config.startScaleMin.y);
	config.startScaleMin.z = std::max(0.0f, config.startScaleMin.z);

	config.startScaleMax.x = std::max(config.startScaleMin.x, config.startScaleMax.x);
	config.startScaleMax.y = std::max(config.startScaleMin.y, config.startScaleMax.y);
	config.startScaleMax.z = std::max(config.startScaleMin.z, config.startScaleMax.z);

	config.endScaleMin.x = std::max(0.0f, config.endScaleMin.x);
	config.endScaleMin.y = std::max(0.0f, config.endScaleMin.y);
	config.endScaleMin.z = std::max(0.0f, config.endScaleMin.z);

	config.endScaleMax.x = std::max(config.endScaleMin.x, config.endScaleMax.x);
	config.endScaleMax.y = std::max(config.endScaleMin.y, config.endScaleMax.y);
	config.endScaleMax.z = std::max(config.endScaleMin.z, config.endScaleMax.z);

	config.maxVelocity.x = std::max(config.minVelocity.x, config.maxVelocity.x);
	config.maxVelocity.y = std::max(config.minVelocity.y, config.maxVelocity.y);
	config.maxVelocity.z = std::max(config.minVelocity.z, config.maxVelocity.z);

	config.maxRotate.x = std::max(config.minRotate.x, config.maxRotate.x);
	config.maxRotate.y = std::max(config.minRotate.y, config.maxRotate.y);
	config.maxRotate.z = std::max(config.minRotate.z, config.maxRotate.z);

	config.maxRotateVelocity.x = std::max(config.minRotateVelocity.x, config.maxRotateVelocity.x);
	config.maxRotateVelocity.y = std::max(config.minRotateVelocity.y, config.maxRotateVelocity.y);
	config.maxRotateVelocity.z = std::max(config.minRotateVelocity.z, config.maxRotateVelocity.z);

	config.startColorMin.x = std::min(config.startColorMin.x, config.startColorMax.x);
	config.startColorMin.y = std::min(config.startColorMin.y, config.startColorMax.y);
	config.startColorMin.z = std::min(config.startColorMin.z, config.startColorMax.z);
	config.startColorMin.w = std::min(config.startColorMin.w, config.startColorMax.w);

	config.startColorMax.x = std::max(config.startColorMin.x, config.startColorMax.x);
	config.startColorMax.y = std::max(config.startColorMin.y, config.startColorMax.y);
	config.startColorMax.z = std::max(config.startColorMin.z, config.startColorMax.z);
	config.startColorMax.w = std::max(config.startColorMin.w, config.startColorMax.w);

	config.endColorMin.x = std::min(config.endColorMin.x, config.endColorMax.x);
	config.endColorMin.y = std::min(config.endColorMin.y, config.endColorMax.y);
	config.endColorMin.z = std::min(config.endColorMin.z, config.endColorMax.z);
	config.endColorMin.w = std::min(config.endColorMin.w, config.endColorMax.w);

	config.endColorMax.x = std::max(config.endColorMin.x, config.endColorMax.x);
	config.endColorMax.y = std::max(config.endColorMin.y, config.endColorMax.y);
	config.endColorMax.z = std::max(config.endColorMin.z, config.endColorMax.z);
	config.endColorMax.w = std::max(config.endColorMin.w, config.endColorMax.w);

	config.boxMin.x = std::min(config.boxMin.x, config.boxMax.x);
	config.boxMin.y = std::min(config.boxMin.y, config.boxMax.y);
	config.boxMin.z = std::min(config.boxMin.z, config.boxMax.z);

	config.boxMax.x = std::max(config.boxMin.x, config.boxMax.x);
	config.boxMax.y = std::max(config.boxMin.y, config.boxMax.y);
	config.boxMax.z = std::max(config.boxMin.z, config.boxMax.z);

	return changed;
#endif
}
