#include "rectangle.h"
#include <algorithm>

namespace Engine {

bool Rectangle::intersects(const Rectangle& other) const {
    return !(right() <= other.left() || 
             left() >= other.right() ||
             bottom() <= other.top() ||
             top() >= other.bottom());
}

bool Rectangle::contains(const sf::Vector2f& point) const {
    return point.x >= left() && point.x <= right() &&
           point.y >= top() && point.y <= bottom();
}

Rectangle Rectangle::getIntersection(const Rectangle& other) const {
    float interLeft = std::max(left(), other.left());
    float interTop = std::max(top(), other.top());
    float interRight = std::min(right(), other.right());
    float interBottom = std::min(bottom(), other.bottom());
    
    if (interLeft < interRight && interTop < interBottom) {
        return Rectangle(
            interLeft, 
            interTop, 
            interRight - interLeft, 
            interBottom - interTop
        );
    }
    
    return Rectangle(); // Empty rectangle (no intersection)
}

} // namespace Engine