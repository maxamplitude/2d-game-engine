#include "camera.h"
#include <cmath>
#include <algorithm>

namespace Engine {

// Lerp helper
static float lerp(float a, float b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

static sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
    return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
}

Camera::Camera() 
    : position(0.0f, 0.0f)
    , size(800.0f, 600.0f)
    , zoom(1.0f)
    , rotation(0.0f)
    , isZooming(false)
    , targetZoom(1.0f)
    , zoomDuration(0.3f)
    , zoomProgress(0.0f)
    , targetPosition(nullptr)
    , followMode(CameraFollowMode::None)
    , followSpeed(5.0f)
    , deadzone(0.0f, 0.0f, 0.0f, 0.0f)
    , trauma(0.0f)
    , traumaDecay(1.0f)
    , shakeIntensity(50.0f)
    , rng(std::random_device{}())
    , randomDist(-1.0f, 1.0f)
    , hasBounds(false)
    , bounds(0.0f, 0.0f, 0.0f, 0.0f)
{
}

Camera::Camera(const sf::Vector2f& position, const sf::Vector2f& size)
    : Camera()  // Delegate to default constructor
{
    this->position = position;
    this->size = size;
}

void Camera::update(float dt) {
    updateFollowing(dt);
    updateZoom(dt);
    updateShake(dt);
    applyBounds();
}

void Camera::setPosition(const sf::Vector2f& pos) {
    position = pos;
}

void Camera::move(const sf::Vector2f& offset) {
    position += offset;
}

void Camera::setSize(const sf::Vector2f& size) {
    this->size = size;
}

void Camera::setZoom(float zoom) {
    this->zoom = std::max(0.1f, zoom);  // Prevent zero/negative zoom
    isZooming = false;
}

void Camera::zoomTo(float targetZoom, float duration) {
    this->targetZoom = std::max(0.1f, targetZoom);
    this->zoomDuration = std::max(0.001f, duration);
    this->zoomProgress = 0.0f;
    this->isZooming = true;
}

void Camera::zoomBy(float factor) {
    setZoom(zoom * factor);
}

void Camera::setRotation(float degrees) {
    rotation = degrees;
}

void Camera::setTarget(const sf::Vector2f* target) {
    targetPosition = target;
}

void Camera::setDeadzone(const sf::FloatRect& zone) {
    deadzone = zone;
}

void Camera::addTrauma(float amount) {
    trauma = std::clamp(trauma + amount, 0.0f, 1.0f);
}

void Camera::setTrauma(float amount) {
    trauma = std::clamp(amount, 0.0f, 1.0f);
}

void Camera::setBounds(const sf::FloatRect& bounds) {
    this->bounds = bounds;
    this->hasBounds = true;
}

sf::View Camera::toSFMLView() const {
    sf::View view;
    
    // Apply shake offset
    sf::Vector2f shakeOffset = calculateShakeOffset();
    sf::Vector2f finalPosition = position + shakeOffset;
    
    view.setCenter(finalPosition);
    view.setSize(size * zoom);
    view.setRotation(rotation);
    
    return view;
}

sf::Vector2f Camera::screenToWorld(const sf::Vector2f& screenPos, 
                                  const sf::Vector2u& windowSize) const {
    // Normalize screen coordinates to [-1, 1] range
    sf::Vector2f normalized;
    normalized.x = (screenPos.x / windowSize.x) * 2.0f - 1.0f;
    normalized.y = (screenPos.y / windowSize.y) * 2.0f - 1.0f;
    
    // Apply zoom and camera position
    sf::Vector2f worldPos;
    worldPos.x = position.x + normalized.x * (size.x * zoom * 0.5f);
    worldPos.y = position.y + normalized.y * (size.y * zoom * 0.5f);
    
    // Note: Rotation not applied in this simple version
    // Full implementation would need rotation matrix
    
    return worldPos;
}

sf::Vector2f Camera::worldToScreen(const sf::Vector2f& worldPos, 
                                  const sf::Vector2u& windowSize) const {
    // Get position relative to camera
    sf::Vector2f relative = worldPos - position;
    
    // Apply zoom
    relative.x /= (size.x * zoom * 0.5f);
    relative.y /= (size.y * zoom * 0.5f);
    
    // Convert from [-1, 1] range to screen coordinates
    sf::Vector2f screenPos;
    screenPos.x = (relative.x + 1.0f) * 0.5f * windowSize.x;
    screenPos.y = (relative.y + 1.0f) * 0.5f * windowSize.y;
    
    return screenPos;
}

sf::FloatRect Camera::getViewBounds() const {
    sf::Vector2f halfSize = size * zoom * 0.5f;
    return sf::FloatRect(
        position.x - halfSize.x,
        position.y - halfSize.y,
        size.x * zoom,
        size.y * zoom
    );
}

bool Camera::isVisible(const sf::FloatRect& rect) const {
    sf::FloatRect viewBounds = getViewBounds();
    return viewBounds.intersects(rect);
}

void Camera::updateFollowing(float dt) {
    if (!targetPosition || followMode == CameraFollowMode::None) {
        return;
    }
    
    switch (followMode) {
        case CameraFollowMode::Instant:
            position = *targetPosition;
            break;
            
        case CameraFollowMode::Smooth: {
            // Smooth lerp following
            float t = followSpeed * dt;
            position = lerp(position, *targetPosition, t);
            break;
        }
            
        case CameraFollowMode::Deadzone: {
            // Only move camera if target exits deadzone
            sf::Vector2f targetLocal = *targetPosition - position;
            
            // Check if target is outside deadzone
            if (targetLocal.x < deadzone.left) {
                position.x += targetLocal.x - deadzone.left;
            } else if (targetLocal.x > deadzone.left + deadzone.width) {
                position.x += targetLocal.x - (deadzone.left + deadzone.width);
            }
            
            if (targetLocal.y < deadzone.top) {
                position.y += targetLocal.y - deadzone.top;
            } else if (targetLocal.y > deadzone.top + deadzone.height) {
                position.y += targetLocal.y - (deadzone.top + deadzone.height);
            }
            break;
        }
            
        default:
            break;
    }
}

void Camera::updateZoom(float dt) {
    if (!isZooming) return;
    
    zoomProgress += dt;
    float t = zoomProgress / zoomDuration;
    
    if (t >= 1.0f) {
        // Zoom complete
        zoom = targetZoom;
        isZooming = false;
    } else {
        // Smooth zoom interpolation (ease-out)
        float smoothT = 1.0f - std::pow(1.0f - t, 3.0f);  // Cubic ease-out
        zoom = lerp(zoom, targetZoom, smoothT);
    }
}

void Camera::updateShake(float dt) {
    if (trauma > 0.0f) {
        trauma = std::max(0.0f, trauma - traumaDecay * dt);
    }
}

void Camera::applyBounds() {
    if (!hasBounds) return;
    
    sf::Vector2f halfSize = size * zoom * 0.5f;
    
    // Clamp camera position to bounds
    if (halfSize.x * 2.0f < bounds.width) {
        position.x = std::clamp(
            position.x,
            bounds.left + halfSize.x,
            bounds.left + bounds.width - halfSize.x
        );
    } else {
        // View is larger than bounds - center it
        position.x = bounds.left + bounds.width * 0.5f;
    }
    
    if (halfSize.y * 2.0f < bounds.height) {
        position.y = std::clamp(
            position.y,
            bounds.top + halfSize.y,
            bounds.top + bounds.height - halfSize.y
        );
    } else {
        // View is larger than bounds - center it
        position.y = bounds.top + bounds.height * 0.5f;
    }
}

sf::Vector2f Camera::calculateShakeOffset() const {
    if (trauma <= 0.0f) {
        return {0.0f, 0.0f};
    }
    
    // Shake intensity based on trauma squared (nice falloff curve)
    float shake = trauma * trauma;
    
    // Generate random offsets
    sf::Vector2f offset;
    offset.x = randomDist(rng) * shake * shakeIntensity;
    offset.y = randomDist(rng) * shake * shakeIntensity;
    
    return offset;
}

} // namespace Engine