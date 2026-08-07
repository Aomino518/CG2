#pragma once
#include "CreateResorceUtils.h"
#include <nlohmann/json.hpp>
#include "JsonTransform.h"

class AccelerationField
{
public:

    void DrawDebug(const Vector3& origin = { 0.0f, 0.0f, 0.0f });

    // Getter
    FieldSpace GetSpace() const { return space_; }
    const Vector3& GetPosition() const { return position_; }
    const Vector3& GetAcceleration() const { return acceleration_; }
    AABB GetAABB() const { return area_; }
    bool GetIsActive() const { return isActive_; }
    AABB GetWorldAABB(const Vector3& origin = { 0.0f, 0.0f, 0.0f }) const;

    // Setter
    void SetSpace(const FieldSpace space) { this->space_ = space; }
    void SetPosition(const Vector3& position) { this->position_ = position; }
    void SetAcceleration(const Vector3& acceleration) { this->acceleration_ = acceleration; }
    void SetAABB(const AABB& aabb) { this->area_ = aabb; }
    void SetIsActive(const bool isActive) { this->isActive_ = isActive; }

    // Json保存と読み込み
    json SaveToJson() const;
    void LoadFromJson(const json& j);

	void ImGuiDraw();

private:
    FieldSpace space_ = FieldSpace::Local;
    Vector3 position_ = { 0.0f, 0.0f, 0.0f };
    Vector3 acceleration_ = { 0.0f, 0.0f, 0.0f };
    AABB area_ = { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f} };
    bool isActive_ = true;
};

