#include "transform.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>

namespace Engine {

Mat4 Transform::toMatrix() const {
    Mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, Vec3(position, 0.0f));
    mat = glm::rotate(mat, toRadians(rotation), Vec3(0.0f, 0.0f, 1.0f));
    mat = glm::scale(mat, Vec3(scale, 1.0f));
    return mat;
}

Vec2 Transform::transformPoint(const Vec2& point) const {
    Mat4 mat = toMatrix();
    Vec4 result = mat * Vec4(point, 0.0f, 1.0f);
    return Vec2(result.x, result.y);
}

Transform Transform::operator*(const Transform& other) const {
    Transform result;
    result.position = position + other.position;
    result.rotation = rotation + other.rotation;
    result.scale = Vec2(scale.x * other.scale.x, scale.y * other.scale.y);
    return result;
}

bool Transform::operator==(const Transform& other) const {
    constexpr float epsilon = 0.001f;

    return glm::all(glm::epsilonEqual(position, other.position, epsilon)) &&
           glm::all(glm::epsilonEqual(scale, other.scale, epsilon)) &&
           std::abs(rotation - other.rotation) < epsilon;
}

} // namespace Engine