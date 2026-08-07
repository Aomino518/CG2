#include "DebugDraw.h"
#include "DebugDraw2D.h"
#include "DebugDraw3D.h"

void DebugDraw::Draw() {
	DebugDraw2D::GetInstance()->Draw();
	DebugDraw3D::GetInstance()->Draw();
}

void DebugDraw::Update() {
	DebugDraw2D::GetInstance()->Update();
	DebugDraw3D::GetInstance()->Update();
}

void DebugDraw::DrawLine(const Vector2& start, const Vector2& end, const Vector4& color)
{
	DebugDraw2D::GetInstance()->DrawLine(start, end, color);
}

void DebugDraw::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	DebugDraw3D::GetInstance()->DrawLine(start, end, color);
}

void DebugDraw::DrawCircle(const Vector2& center, float radiusX, float radiusY, const Vector4& color, DebugDrawMode mode)
{
	if (mode == DebugDrawMode::Solid) {
		DebugDraw2D::GetInstance()->DrawCircleSolid(center, radiusX, radiusY, color);
	} else if (mode == DebugDrawMode::Wireframe) {
		DebugDraw2D::GetInstance()->DrawCircleWire(center, radiusX, radiusY, color);
	}
}

void DebugDraw::DrawSphere(const Vector3& center, const Vector3& radius, const Vector4& color, DebugDrawMode mode)
{
	if (mode == DebugDrawMode::Solid) {
		DebugDraw3D::GetInstance()->DrawSphereSolid(center, radius, color);
	} else if (mode == DebugDrawMode::Wireframe) {
		DebugDraw3D::GetInstance()->DrawSphereWire(center, radius, color);
	}
}

void DebugDraw::DrawBox(const Vector2& center, const Vector2& halfSize, const Vector4& color, DebugDrawMode mode)
{
	if (mode == DebugDrawMode::Solid) {
		DebugDraw2D::GetInstance()->DrawBoxSolid(center, halfSize, color);
	} else if (mode == DebugDrawMode::Wireframe) {
		DebugDraw2D::GetInstance()->DrawBoxWire(center, halfSize, color);
	}
}

void DebugDraw::DrawRect(const Vector2& leftTop, const Vector2& size, const Vector4& color, DebugDrawMode mode)
{
	if (mode == DebugDrawMode::Solid) {
		DebugDraw2D::GetInstance()->DrawRectSolid(leftTop, size, color);
	} else if (mode == DebugDrawMode::Wireframe) {
		DebugDraw2D::GetInstance()->DrawRectWire(leftTop, size, color);
	}
}

void DebugDraw::DrawAABB(const Vector3& position, const AABB& aabb, const Vector4& color, DebugDrawMode mode)
{
	if (mode == DebugDrawMode::Solid) {
		DebugDraw3D::GetInstance()->DrawAABBSolid(position, aabb, color);
	} else if (mode == DebugDrawMode::Wireframe) {
		DebugDraw3D::GetInstance()->DrawAABBWire(position, aabb, color);
	}
}

void DebugDraw::DrawAABB2D(const Vector2& position, const AABB2D& aabb, const Vector4& color, DebugDrawMode mode)
{
	if (mode == DebugDrawMode::Solid) {
		DebugDraw2D::GetInstance()->DrawAABB2DSolid(position, aabb, color);
	} else if (mode == DebugDrawMode::Wireframe) {
		DebugDraw2D::GetInstance()->DrawAABB2DWire(position, aabb, color);
	}
}

void DebugDraw::DrawBox(const Vector3& center, const Vector3& halfSize, const Vector4& color, DebugDrawMode mode)
{
	if (mode == DebugDrawMode::Solid) {
		DebugDraw3D::GetInstance()->DrawBoxSolid(center, halfSize, color);
	} else if (mode == DebugDrawMode::Wireframe) {
		DebugDraw3D::GetInstance()->DrawBoxWire(center, halfSize, color);
	}
}