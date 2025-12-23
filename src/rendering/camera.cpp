#include "camera.h"
#include <algorithm>
#include <cmath>

namespace Engine {

static Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
    return a + (b - a) * t;
}

Camera::Camera()
    : position(0, 0), size(800, 600), zoom(1.0f), rotation(0.0f),
      isZooming(false), targetZoom(1.0f), zoomDuration(0.3f), zoomProgress(0.0f),
      targetPosition(nullptr), followMode(CameraFollowMode::None), followSpeed(5.0f),
      trauma(0.0f), traumaDecay(1.5f), shakeIntensity(5.0f),
      rng(std::random_device{}()), randomDist(-1.0f, 1.0f),
      hasBounds(false) {}

Camera::Camera(const Vec2& position, const Vec2& size)
    : position(position), size(size), zoom(1.0f), rotation(0.0f),
      isZooming(false), targetZoom(1.0f), zoomDuration(0.3f), zoomProgress(0.0f),
      targetPosition(nullptr), followMode(CameraFollowMode::None), followSpeed(5.0f),
      trauma(0.0f), traumaDecay(1.5f), shakeIntensity(5.0f),
      rng(std::random_device{}()), randomDist(-1.0f, 1.0f),
      hasBounds(false) {}

void Camera::update(float dt) {
    updateFollowing(dt);
    updateZoom(dt);
    updateShake(dt);
    applyBounds();
}

void Camera::setPosition(const Vec2& pos) { position = pos; }
void Camera::move(const Vec2& offset) { position += offset; }
void Camera::setSize(const Vec2& s) { size = s; }

void Camera::setZoom(float z) {
    zoom = std::max(0.1f, z);
    isZooming = false;
}

void Camera::zoomTo(float z, float duration) {
    targetZoom = std::max(0.1f, z);
    zoomDuration = duration;
    zoomProgress = 0.0f;
    isZooming = true;
}

void Camera::zoomBy(float factor) { setZoom(zoom * factor); }
void Camera::setRotation(float degrees) { rotation = degrees; }
void Camera::setTarget(const Vec2* target) { targetPosition = target; }
void Camera::setDeadzone(const Rectangle& zone) { deadzone = zone; }

void Camera::addTrauma(float amount) { trauma = std::clamp(trauma + amount, 0.0f, 1.0f); }
void Camera::setTrauma(float amount) { trauma = std::clamp(amount, 0.0f, 1.0f); }
void Camera::setBounds(const Rectangle& b) { bounds = b; hasBounds = true; }

Rectangle Camera::getViewBounds() const {
    Vec2 halfSize = size * zoom * 0.5f;
    return Rectangle(position.x - halfSize.x,
                     position.y - halfSize.y,
                     halfSize.x * 2.0f,
                     halfSize.y * 2.0f);
}

bool Camera::isVisible(const Rectangle& rect) const {
    return getViewBounds().intersects(rect);
}

Mat4 Camera::getViewMatrix() const {
    Mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, Vec3(-position, 0.0f));
    view = glm::rotate(view, -toRadians(rotation), Vec3(0.0f, 0.0f, 1.0f));
    view = glm::scale(view, Vec3(1.0f / zoom, 1.0f / zoom, 1.0f));
    return view;
}

Mat4 Camera::getProjection(float viewportWidth, float viewportHeight) const {
    return glm::ortho(0.0f, viewportWidth, viewportHeight, 0.0f, -1.0f, 1.0f);
}

void Camera::updateFollowing(float dt) {
    if (!targetPosition) return;

    switch (followMode) {
        case CameraFollowMode::Instant:
            position = *targetPosition;
            break;
        case CameraFollowMode::Smooth:
            position = lerp(position, *targetPosition, 1.0f - std::exp(-followSpeed * dt));
            break;
        case CameraFollowMode::Deadzone: {
            Rectangle dz = deadzone;
            dz.x += position.x;
            dz.y += position.y;
            if (!dz.contains(*targetPosition)) {
                position = lerp(position, *targetPosition, 1.0f - std::exp(-followSpeed * dt));
            }
            break;
        }
        case CameraFollowMode::None:
        default:
            break;
    }
}

void Camera::updateZoom(float dt) {
    if (!isZooming) return;

    zoomProgress += dt;
    float t = std::min(zoomProgress / zoomDuration, 1.0f);
    float smoothT = t * t * (3.0f - 2.0f * t);
    zoom = zoom + (targetZoom - zoom) * smoothT;

    if (t >= 1.0f) {
        zoom = targetZoom;
        isZooming = false;
    }
}

void Camera::updateShake(float dt) {
    if (trauma <= 0.0f) return;
    trauma = std::max(0.0f, trauma - traumaDecay * dt);
}

void Camera::applyBounds() {
    if (!hasBounds) return;
    Vec2 halfSize = size * zoom * 0.5f;
    position.x = std::clamp(position.x, bounds.left() + halfSize.x, bounds.right() - halfSize.x);
    position.y = std::clamp(position.y, bounds.top() + halfSize.y, bounds.bottom() - halfSize.y);
}

Vec2 Camera::calculateShakeOffset() const {
    if (trauma <= 0.0f) return Vec2(0.0f);
    float shake = trauma * trauma;
    return Vec2(randomDist(rng) * shakeIntensity * shake,
                randomDist(rng) * shakeIntensity * shake);
}

} // namespace Engine

