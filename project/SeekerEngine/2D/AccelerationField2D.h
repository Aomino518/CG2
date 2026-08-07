#pragma once
#include "CreateResorceUtils.h"
#include <nlohmann/json.hpp>
#include "JsonTransform.h"

class AccelerationField2D
{
public:
    void DrawDebug(const Vector2& origin = { 0.0f, 0.0f });

    // Getter
    FieldSpace GetSpace() const { return space_; }
    const Vector2& GetPosition() const { return position_; }
    const Vector2& GetAcceleration() const { return acceleration_; }
    AABB2D GetAABB() const { return area_; }
    bool GetIsActive() const { return isActive_; }
    AABB2D GetWorldAABB(const Vector2& origin = { 0.0f, 0.0f }) const;

    // Setter
    void SetSpace(const FieldSpace space) { this->space_ = space; }
    void SetPosition(const Vector2& position) { this->position_ = position; }
    void SetAcceleration(const Vector2& acceleration) { this->acceleration_ = acceleration; }
    void SetAABB2D(const AABB2D& aabb) { this->area_ = aabb; }
    void SetIsActive(const bool isActive) { this->isActive_ = isActive; }

    // Json保存と読み込み
    nlohmann::json SaveToJson() const;
    void LoadFromJson(const nlohmann::json& j);

    void ImGuiDraw();

private:
    FieldSpace space_ = FieldSpace::Local;
    Vector2 position_ = { 0.0f, 0.0f };
    Vector2 acceleration_ = { 0.0f, 0.0f };
    AABB2D area_ = { { -32.0f, -32.0f }, { 32.0f, 32.0f } };
    bool isActive_ = true;
};

