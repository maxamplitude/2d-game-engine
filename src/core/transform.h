#pragma once
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>

namespace Engine {

struct Transform {
    sf::Vector2f position{0.0f, 0.0f};
    sf::Vector2f scale{1.0f, 1.0f};
    float rotation = 0.0f;  // Degrees
    
    // Convert to SFML transform for rendering
    sf::Transform toSFMLTransform() const;
    
    // Transform a point by this transform
    sf::Vector2f transformPoint(const sf::Vector2f& point) const;
    
    // Combine transforms
    Transform operator*(const Transform& other) const;
    
    // Equality comparison (with epsilon for floats)
    bool operator==(const Transform& other) const;
    bool operator!=(const Transform& other) const { return !(*this == other); }
};

} // namespace Engine