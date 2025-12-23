#pragma once
#include "math/vector.h"
#include "math/rectangle.h"
#include <random>

namespace Engine {

// Camera follow modes
enum class CameraFollowMode {
    None,           // No following
    Instant,        // Snap to target instantly
    Smooth,         // Smooth lerp following
    Deadzone        // Only move when target exits deadzone
};

class Camera {
public:
    Camera();
    Camera(const Vec2& position, const Vec2& size);
    
    // Update (call every frame)
    void update(float dt);
    
    // Position
    void setPosition(const Vec2& pos);
    Vec2 getPosition() const { return position; }
    void move(const Vec2& offset);
    
    // Size (viewport dimensions in world units)
    void setSize(const Vec2& size);
    Vec2 getSize() const { return size; }
    
    // Zoom
    void setZoom(float zoom);
    float getZoom() const { return zoom; }
    void zoomTo(float targetZoom, float duration = 0.3f);  // Smooth zoom transition
    void zoomBy(float factor);  // Multiply current zoom
    
    // Rotation (in degrees)
    void setRotation(float degrees);
    float getRotation() const { return rotation; }
    
    // Target following
    void setTarget(const Vec2* target);
    void clearTarget() { targetPosition = nullptr; }
    void setFollowMode(CameraFollowMode mode) { followMode = mode; }
    void setFollowSpeed(float speed) { followSpeed = speed; }  // For smooth following
    
    // Deadzone (for deadzone follow mode)
    void setDeadzone(const Rectangle& zone);
    Rectangle getDeadzone() const { return deadzone; }
    
    // Screen shake (trauma-based system)
    void addTrauma(float amount);  // 0.0 to 1.0, trauma decays over time
    void setTrauma(float amount);
    float getTrauma() const { return trauma; }
    void setShakeIntensity(float intensity) { shakeIntensity = intensity; }
    void setTraumaDecay(float decay) { traumaDecay = decay; }
    
    // Bounds (optional - keeps camera within world boundaries)
    void setBounds(const Rectangle& bounds);
    void clearBounds() { hasBounds = false; }
    bool isWithinBounds() const { return hasBounds; }
    
    Rectangle getViewBounds() const;
    bool isVisible(const Rectangle& rect) const;

    Mat4 getViewMatrix() const;
    Mat4 getProjection(float viewportWidth, float viewportHeight) const;
    
private:
    // Core properties
    Vec2 position;
    Vec2 size;  // View size in world units
    float zoom;
    float rotation;  // Degrees
    
    // Smooth zoom
    bool isZooming;
    float targetZoom;
    float zoomDuration;
    float zoomProgress;
    
    // Following
    const Vec2* targetPosition;
    CameraFollowMode followMode;
    float followSpeed;  // For smooth following
    Rectangle deadzone;  // For deadzone following
    
    // Screen shake
    float trauma;  // 0.0 to 1.0
    float traumaDecay;  // Units per second
    float shakeIntensity;  // Max shake offset in pixels
    mutable std::mt19937 rng;
    mutable std::uniform_real_distribution<float> randomDist;
    
    // Bounds
    bool hasBounds;
    Rectangle bounds;
    
    // Helper methods
    void updateFollowing(float dt);
    void updateZoom(float dt);
    void updateShake(float dt);
    void applyBounds();
    Vec2 calculateShakeOffset() const;
};

} // namespace Engine