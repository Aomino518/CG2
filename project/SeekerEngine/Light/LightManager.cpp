#include "LightManager.h"
#include "Logger.h"
#include <numbers>
#include <nlohmann/json.hpp>
#include "JsonTransform.h"
#include "Graphics.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif

LightManager* LightManager::GetInstance()
{
    static LightManager instance;
    return &instance;
}

void LightManager::Init()
{
    // 平行光源用のリソース
    directionalLightResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(DirectionalLight));
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&dirLight_));
    // 初期化値
    dirLight_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    dirLight_->direction = { 1.0f, 0.0f, 0.0f };
    dirLight_->intensity = 0.0f;

    pointLightGroupResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(PointLightGroup));
    pointLightGroupResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightGroup_));
    // 明示的にメモリを0クリア
    memset(pointLightGroup_, 0, sizeof(PointLightGroup));

    spotLightGroupResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(SpotLightGroup));
    spotLightGroupResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightGroup_));
    // 明示的にメモリを0クリア
    memset(spotLightGroup_, 0, sizeof(SpotLightGroup));
}

void LightManager::Update()
{
    uint32_t pointCount = 0;
    for (auto& [name, entry] : pointLights_) {
        if (pointCount >= kMaxPointLights) { break; }
        pointLightGroup_->lights[pointCount] = entry;
        ++pointCount;
    }
    pointLightGroup_->count = static_cast<int32_t>(pointCount);

    for (uint32_t i = pointCount; i < kMaxPointLights; ++i) {
        pointLightGroup_->lights[i] = {};
    }

    uint32_t spotCount = 0;
    for (auto& [name, entry] : spotLights_) {
        if (spotCount >= kMaxSpotLights) { break; }
        spotLightGroup_->lights[spotCount] = entry;
        ++spotCount;
    }
    spotLightGroup_->count = static_cast<int32_t>(spotCount);

    for (uint32_t i = spotCount; i < kMaxSpotLights; ++i) {
        spotLightGroup_->lights[i] = {};
    }
}

void LightManager::Shutdown()
{
    directionalLightResource_.Reset();
    pointLightGroupResource_.Reset();
    spotLightGroupResource_.Reset();

    pointLights_.clear();
    spotLights_.clear();
    Logger::Write("LightManager Shutdown");
}

void LightManager::CreatePointLight(const std::string& name)
{
    // 同名がないかチェック
    if (pointLights_.find(name) != pointLights_.end()) {
        Logger::Write(Logger::LogLevel::Warning, "PointLight already exists: " + name);
        return;
    }

    PointLight entry{};
    // 初期化値
    entry.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    entry.position = { 0.0f, 2.0f, 2.0f };
    entry.intensity = 1.0f;
    entry.radius = 5.0f;
    entry.decay = 1.0f;

    pointLights_.emplace(name, std::move(entry));
}

void LightManager::CreateSpotLight(const std::string& name)
{
    // 同名がないかチェック
    if (spotLights_.find(name) != spotLights_.end()) {
        Logger::Write(Logger::LogLevel::Warning, "SpotLight already exists: " + name);
        return;
    }

    SpotLight entry{};
    // スポットライトのリリース
    entry.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    entry.position = { -10.0f, 1.0f, 0.0f };
    entry.intensity = 15.0f;
    entry.decay = 2.0f;
    entry.cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    entry.cosFalloffStart = 1.0f;
    entry.direction = Normalize({ -1.0f, -1.0f, 0.0f });
    entry.distance = 20.0f;

    spotLights_.emplace(name, std::move(entry));
}

void LightManager::RemovePointLight(const std::string& name)
{
    auto it = pointLights_.find(name);
    if (it == pointLights_.end()) {
        return;
    }

    pointLights_.erase(it);
}

void LightManager::RemoveSpotLight(const std::string& name)
{
    auto it = spotLights_.find(name);
    if (it == spotLights_.end()) {
        return;
    }

    spotLights_.erase(it);
}

void LightManager::ClearPointLights()
{
    pointLights_.clear();
}

void LightManager::ClearSpotLights()
{
    spotLights_.clear();
}

PointLight* LightManager::GetPointLight(const std::string& name)
{
    auto it = pointLights_.find(name);
    if (it == pointLights_.end()) {
        return nullptr;
    }
    return &(it->second);
}

SpotLight* LightManager::GetSpotLight(const std::string& name)
{
    auto it = spotLights_.find(name);
    if (it == spotLights_.end()) {
        return nullptr;
    }
    return &(it->second);
}

void LightManager::SetDirectionalLight(DirectionalLight* dirLight)
{
    dirLight_->direction = dirLight->direction;
    dirLight_->color = dirLight->color;
    dirLight_->intensity = dirLight->intensity;
}

void LightManager::SetPointLight(std::string& name, PointLight* pointLight)
{
    auto it = pointLights_.find(name);
    if (it == pointLights_.end()) {
        Logger::Write(Logger::LogLevel::Warning, name + "is not PointLight");
        return;
    }

    it->second.position = pointLight->position;
    it->second.color = pointLight->color;
    it->second.intensity = pointLight->intensity;
    it->second.radius = pointLight->radius;
    it->second.decay = pointLight->decay;
}

void LightManager::SetSpotLight(std::string& name, SpotLight* spotLight)
{
    auto it = spotLights_.find(name);
    if (it == spotLights_.end()) {
        Logger::Write(Logger::LogLevel::Warning, name + "is not SpotLight");
        return;
    }

    it->second.position = spotLight->position;
    it->second.color = spotLight->color;
    it->second.intensity = spotLight->intensity;
    it->second.distance = spotLight->distance;
    it->second.decay = spotLight->decay;
    it->second.direction = spotLight->direction;
    it->second.cosAngle = spotLight->cosAngle;
    it->second.cosFalloffStart = spotLight->cosFalloffStart;
}

json LightManager::SaveToJson() const {
    json j;

    // Directional
    j["directionalLight"]["color"] = ToJson(dirLight_->color);
    j["directionalLight"]["direction"] = ToJson(dirLight_->direction);
    j["directionalLight"]["intensity"] = dirLight_->intensity;

    // Point
    for (auto& [name, light] : pointLights_) {
        auto& pointLight = j["pointLights"][name];

        pointLight["position"] = ToJson(light.position);
        pointLight["color"] = ToJson(light.color);
        pointLight["radius"] = light.radius;
        pointLight["decay"] = light.decay;
        pointLight["intensity"] = light.intensity;
    }

    for (auto& [name, light] : spotLights_) {
        auto& spotLight = j["spotLights"][name];

        spotLight["position"] = ToJson(light.position);
        spotLight["color"] = ToJson(light.color);
        spotLight["direction"] = ToJson(light.direction);
        spotLight["distance"] = light.distance;
        spotLight["intensity"] = light.intensity;
        spotLight["cosAngle"] = light.cosAngle;
        spotLight["cosFalloffStart"] = light.cosFalloffStart;
        spotLight["decay"] = light.decay;
    }

    return j;
}

void LightManager::LoadFromJson(const json& j) {
   
    if (j.contains("directionalLight")) {
        const auto& data = j.at("directionalLight");
        FromJson(data.at("color"), dirLight_->color);
        FromJson(data.at("direction"), dirLight_->direction);
        dirLight_->intensity = data.at("intensity");
    }

    if (j.contains("pointLights")) {
        for (auto& [name, data] : j["pointLights"].items()) {
            auto* pointLight = GetPointLight(name);
            if (!pointLight) {
                continue; 
            }
            FromJson(data.at("position"), pointLight->position);
            FromJson(data.at("color"), pointLight->color);
            pointLight->radius = data.value("radius", pointLight->radius);
            pointLight->decay = data.value("decay", pointLight->decay);
            pointLight->intensity = data.value("intensity", pointLight->intensity);
        }
    }

    if (j.contains("spotLights")) {
        for (auto& [name, data] : j["spotLights"].items()) {
            auto* spotLight = GetSpotLight(name);
            if (!spotLight) {
                continue;
            }
            FromJson(data.at("position"), spotLight->position);
            FromJson(data.at("color"), spotLight->color);
            FromJson(data.at("direction"), spotLight->direction);
            spotLight->distance = data.value("distance", spotLight->distance);
            spotLight->intensity = data.value("intensity", spotLight->intensity);
            spotLight->cosAngle = data.value("cosAngle", spotLight->cosAngle);
            spotLight->cosFalloffStart = data.value("cosFalloffStart", spotLight->cosFalloffStart);
            spotLight->decay = data.value("decay", spotLight->decay);
        }
    }
}

void LightManager::DrawDirectionalLightImGui(const std::string& name)
{
#ifdef USE_IMGUI
    static float yaw = 0.0f;
    static float pitch = -45.0f;

    ImGui::Text("Name: %s", name.c_str());
    bool changed = false;
    changed |= ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f, "%.1f deg");
    changed |= ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f, "%.1f deg");
    changed |= ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&dirLight_->color));
    changed |= ImGui::DragFloat("Intensity", &dirLight_->intensity, 0.01f, 0.0f, 10.0f, "%.2f");

    if (changed) {
        float yawRad = yaw * std::numbers::pi_v<float> / 180.0f;
        float pitchRad = pitch * std::numbers::pi_v<float> / 180.0f;

        dirLight_->direction.x = cosf(pitchRad) * cosf(yawRad);
        dirLight_->direction.y = sinf(pitchRad);
        dirLight_->direction.z = cosf(pitchRad) * sinf(yawRad);

        SetDirectionalLight(dirLight_);
    }
#endif
}

void LightManager::DrawPointLightImGui(const std::string& name)
{
#ifdef USE_IMGUI
    auto it = pointLights_.find(name);
    if (it != pointLights_.end()) {
        ImGui::Text("Name: %s", it->first.c_str());
        bool changed = false;
        changed |= ImGui::DragFloat3("Position", (float*)&it->second.position, 0.01f, -100.0f, 100.0f, "%.2f");
        changed |= ImGui::ColorEdit4("Color", (float*)&it->second.color);
        changed |= ImGui::DragFloat("Intensity", &it->second.intensity, 0.01f, 0.0f, 10.0f, "%.2f");
        changed |= ImGui::DragFloat("Radius", &it->second.radius, 0.01f, 0.0f, 100.0f, "%.2f");
        changed |= ImGui::DragFloat("Decay", &it->second.decay, 0.01f, 0.0f, 10.0f, "%.2f");

        if (changed) {
            std::string lightName = name;
            SetPointLight(lightName, &it->second);
        }
    }
#endif
}

void LightManager::DrawSpotLightImGui(const std::string& name)
{
#ifdef USE_IMGUI
    auto it = spotLights_.find(name);
    if (it != spotLights_.end()) {
        ImGui::Text("Name: %s", it->first.c_str());
        bool changed = false;
        changed |= ImGui::DragFloat3("Position", (float*)&it->second.position, 0.01f, -100.0f, 100.0f, "%.2f");
        changed |= ImGui::ColorEdit4("Color", (float*)&it->second.color);
        changed |= ImGui::DragFloat("Intensity", &it->second.intensity, 0.01f, 0.0f, 100.0f, "%.2f");
        changed |= ImGui::DragFloat("Distance", &it->second.distance, 0.01f, 0.0f, 100.0f, "%.2f");
        changed |= ImGui::DragFloat("Decay", &it->second.decay, 0.01f, 0.0f, 10.0f, "%.2f");
        changed |= ImGui::DragFloat("CosAngle", &it->second.cosAngle, 0.01f, -1.0f, 1.0f, "%.2f");
        changed |= ImGui::DragFloat("CosFalloffStart", &it->second.cosFalloffStart, 0.01f, -1.0f, 1.0f, "%.2f");
        changed |= ImGui::DragFloat3("Direction", (float*)&it->second.direction, 0.01f, -1.0f, 1.0f, "%.2f");

        if (changed) {
            it->second.direction = Normalize(it->second.direction);
            std::string lightName = name;
            SetSpotLight(lightName, &it->second);
        }
    }
#endif
}
