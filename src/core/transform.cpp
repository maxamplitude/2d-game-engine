#include "transform.h"
#include <cmath>

namespace Engine {

sf::Transform Transform::toSFMLTransform() const {
    sf::Transform t;
    t.translate(position);
    t.rotate(rotation);
    t.scale(scale);
    return t;
}

sf::Vector2f Transform::transformPoint(const sf::Vector2f& point) const {
    return toSFMLTransform().transformPoint(point);
}

Transform Transform::operator*(const Transform& other) const {
    Transform result;
    result.position = position + other.position;
    result.rotation = rotation + other.rotation;
    result.scale = {scale.x * other.scale.x, scale.y * other.scale.y};
    return result;
}

bool Transform::operator==(const Transform& other) const {
    constexpr float epsilon = 0.001f;
    
    return std::abs(position.x - other.position.x) < epsilon &&
           std::abs(position.y - other.position.y) < epsilon &&
           std::abs(scale.x - other.scale.x) < epsilon &&
           std::abs(scale.y - other.scale.y) < epsilon &&
           std::abs(rotation - other.rotation) < epsilon;
}

} // namespace Engine