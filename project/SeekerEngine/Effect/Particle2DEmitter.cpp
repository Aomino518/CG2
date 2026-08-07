#include "Particle2DEmitter.h"
#include "ImGuiUtils.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

Particle2DEmitter::Particle2DEmitter(const std::string& groupName, 
	const ParticleConfig& config, 
	uint32_t count, 
	float frequency) 
	: groupName_(groupName),
	config_(config),
	count_(count),
	frequency_(frequency),
	frequencyTime_(0.0f)
{
	transform_.translate = { 640.0f, 360.0f, 0.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.scale = { 32.0f, 32.0f, 0.0f };
}

void Particle2DEmitter::EmitOnce()
{
	Particle2DManager::GetInstance()->Emit(
		groupName_,
		config_,
		transform_.translate,
		count_);
}

void Particle2DEmitter::StartLoop()
{
	isLoop_ = true;
	frequencyTime_ = 0.0f;
}

void Particle2DEmitter::StopLoop()
{
	isLoop_ = false;
}

void Particle2DEmitter::Update()
{
	if (isLoop_) {
		frequencyTime_ += kDeltaTime;

		while (frequency_ <= frequencyTime_) {
			Particle2DManager::GetInstance()->Emit(
				groupName_,
				config_,
				transform_.translate,
				count_);
			frequencyTime_ -= frequency_;
		}
	}
}

void Particle2DEmitter::DrawDebug()
{
#ifdef _DEBUG
	localField_.DrawDebug({ transform_.translate.x, transform_.translate.y });
#endif
}

void Particle2DEmitter::SetSpawnShapeBox(const Vector2& min, const Vector2& max)
{
	config_.shape = SpawnShape::Box;
	config_.boxMin = { min.x, min.y, 0.0f };
	config_.boxMax = { max.x, max.y, 0.0f };
}

void Particle2DEmitter::SetSpawnShapeSphere(float radius)
{
	config_.shape = SpawnShape::Sphere;
	config_.sphereRadius = radius;
}

void Particle2DEmitter::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Text("Emitter: %s", groupName_.c_str());

	// =========================
	// Transform
	// =========================
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat2("Position", &transform_.translate.x, 0.1f);
		ImGui::DragFloat("Rotation", &transform_.rotate.z, 0.1f);
		ImGui::DragFloat2("Scale", &transform_.scale.x, 0.1f);
	}

	// =========================
	// Emit Control
	// =========================
	if (ImGui::CollapsingHeader("Emit Control", ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::DragInt("Count", reinterpret_cast<int*>(&count_), 1, 1, 1000);
		ImGui::DragFloat("Frequency", &frequency_, 0.01f, 0.01f, 10.0f);

		if (ImGui::Button("Emit Once")) {
			EmitOnce();
		}

		if (ImGui::Button(isLoop_ ? "Stop Loop" : "Start Loop")) {
			if (isLoop_) {
				StopLoop();
			} else {
				StartLoop();
			}
		}
	}

	// =========================
	// LocalField Control
	// =========================
	if (ImGui::CollapsingHeader("LocalField", ImGuiTreeNodeFlags_DefaultOpen)) {
		localField_.ImGuiDraw();
	}

	ParticleConfig config = config_;
	if (ImGuiUtils::DrawEditParticleConfig2D(config)) {
		SetConfig(config);
	}
#endif
}

json Particle2DEmitter::SaveToJson() const
{
	return json{
		{"transform", TransformToJson(transform_)},
		{"count", count_},
		{"frequency", frequency_},
		{"loop", isLoop_},
		{"localField", localField_.SaveToJson()},
		// Spawn Shape
		 {"shape", static_cast<int>(config_.shape)},
		 {"boxMin", ToJson(config_.boxMin)},
		 {"boxMax", ToJson(config_.boxMax)},
		 {"sphereRadius", config_.sphereRadius},

		 // Velocity
		 {"minVelocity", ToJson(config_.minVelocity)},
		 {"maxVelocity", ToJson(config_.maxVelocity)},

		 // Rotation
		 {"minRotate", ToJson(config_.minRotate)},
		 {"maxRotate", ToJson(config_.maxRotate)},
		 {"minRotateVelocity", ToJson(config_.minRotateVelocity)},
		 {"maxRotateVelocity", ToJson(config_.maxRotateVelocity)},

		 // Color
		 {"startColorMin", ToJson(config_.startColorMin)},
		 {"startColorMax", ToJson(config_.startColorMax)},
		 {"endColorMin", ToJson(config_.endColorMin)},
		 {"endColorMax", ToJson(config_.endColorMax)},

		 // Scale
		 {"startScaleMin", ToJson(config_.startScaleMin)},
		 {"startScaleMax", ToJson(config_.startScaleMax)},
		 {"endScaleMin", ToJson(config_.endScaleMin)},
		 {"endScaleMax", ToJson(config_.endScaleMax)},

		 // Lifetime
		 {"minLifeTime", config_.minLifeTime},
		 {"maxLifeTime", config_.maxLifeTime}
	};
}

void Particle2DEmitter::LoadFromJson(const json& j)
{
	if (j.contains("transform")) {
		TransformFromJson(j.at("transform"), transform_);
	}

	if (j.contains("count")) {
		SetCount(j.value("count", 10));
	}

	if (j.contains("frequency")) {
		SetFrenquency(j.value("frequency", 0.1f));
	}

	if (j.contains("loop")) {
		bool loop = j.value("loop", false);
		if (loop) {
			StartLoop();
		} else {
			StopLoop();
		}
	}

	if (j.contains("localField")) {
		localField_.LoadFromJson(j.at("localField"));
	}

	if (j.contains("shape")) {
		config_.shape = static_cast<SpawnShape>(j.value("shape", config_.shape));
	}

	if (j.contains("boxMin")) {
		FromJson(j.at("boxMin"), config_.boxMin);
	}

	if (j.contains("boxMax")) {
		FromJson(j.at("boxMax"), config_.boxMax);
	}

	if (j.contains("sphereRadius")) {
		config_.sphereRadius = j.value("sphereRadius", config_.sphereRadius);
	}

	// Velocity
	if (j.contains("minVelocity")) {
		FromJson(j.at("minVelocity"), config_.minVelocity);
	}

	if (j.contains("maxVelocity")) {
		FromJson(j.at("maxVelocity"), config_.maxVelocity);
	}

	// Rotation
	if (j.contains("minRotate")) {
		FromJson(j.at("minRotate"), config_.minRotate);
	}

	if (j.contains("maxRotate")) {
		FromJson(j.at("maxRotate"), config_.maxRotate);
	}

	if (j.contains("minRotateVelocity")) {
		FromJson(j.at("minRotateVelocity"), config_.minRotateVelocity);
	}

	if (j.contains("maxRotateVelocity")) {
		FromJson(j.at("maxRotateVelocity"), config_.maxRotateVelocity);
	}

	// Color
	if (j.contains("startColorMin")) {
		FromJson(j.at("startColorMin"), config_.startColorMin);
	}

	if (j.contains("startColorMax")) {
		FromJson(j.at("startColorMax"), config_.startColorMax);
	}

	if (j.contains("endColorMin")) {
		FromJson(j.at("endColorMin"), config_.endColorMin);
	}

	if (j.contains("endColorMax")) {
		FromJson(j.at("endColorMax"), config_.endColorMax);
	}

	// Scale
	if (j.contains("startScaleMin")) {
		FromJson(j.at("startScaleMin"), config_.startScaleMin);
	}

	if (j.contains("startScaleMax")) {
		FromJson(j.at("startScaleMax"), config_.startScaleMax);
	}

	if (j.contains("endScaleMin")) {
		FromJson(j.at("endScaleMin"), config_.endScaleMin);
	}

	if (j.contains("endScaleMax")) {
		FromJson(j.at("endScaleMax"), config_.endScaleMax);
	}

	// Lifetime
	if (j.contains("minLifeTime")) {
		config_.minLifeTime = j.value("minLifeTime", config_.minLifeTime);
	}

	if (j.contains("maxLifeTime")) {
		config_.maxLifeTime = j.value("maxLifeTime", config_.maxLifeTime);
	}
}
