#pragma once
#include "Vector2.h"
#include "Vector3.h"

struct AABB {
    Vector3 min;
    Vector3 max;
};

struct AABB2D {
    Vector2 min;
    Vector2 max;
};