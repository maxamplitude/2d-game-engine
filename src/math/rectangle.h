#pragma once
#include <SFML/System/Vector2.hpp>

namespace Engine {

struct Rectangle {
    float x, y, width, height;
    
    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(float x, float y, float w, float h) 
        : x(x), y(y), width(w), height(h) {}
    
    // Boundary accessors
    float left() const { return x; }
    float right() const { return x + width; }
    float top() const { return y; }
    float bottom() const { return y + height; }
    
    // Center point
    sf::Vector2f center() const {
        return {x + width * 0.5f, y + height * 0.5f};
    }
    
    // Collision detection
    bool intersects(const Rectangle& other) const;
    bool contains(const sf::Vector2f& point) const;
    Rectangle getIntersection(const Rectangle& other) const;
    
    // Utility
    float area() const { return width * height; }
    bool isEmpty() const { return width <= 0.0f || height <= 0.0f; }
};

} // namespace Engine