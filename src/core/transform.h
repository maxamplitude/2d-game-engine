#pragma once
#include "math/vector.h"
#include "core/types.h"

namespace Engine {

struct Transform {
    Vec2 position{0.0f, 0.0f};
    Vec2 scale{1.0f, 1.0f};
    float rotation = 0.0f;  // Degrees
    float depth = 0.0f;

    Mat4 toMatrix() const;
    Vec2 transformPoint(const Vec2& point) const;

    Transform operator*(const Transform& other) const;
    bool operator==(const Transform& other) const;
    bool operator!=(const Transform& other) const { return !(*this == other); }
};

} // namespace Engine