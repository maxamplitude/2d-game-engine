#pragma once
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
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
    Camera(const sf::Vector2f& position, const sf::Vector2f& size);
    
    // Update (call every frame)
    void update(float dt);
    
    // Position
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const { return position; }
    void move(const sf::Vector2f& offset);
    
    // Size (viewport dimensions in world units)
    void setSize(const sf::Vector2f& size);
    sf::Vector2f getSize() const { return size; }
    
    // Zoom
    void setZoom(float zoom);
    float getZoom() const { return zoom; }
    void zoomTo(float targetZoom, float duration = 0.3f);  // Smooth zoom transition
    void zoomBy(float factor);  // Multiply current zoom
    
    // Rotation (in degrees)
    void setRotation(float degrees);
    float getRotation() const { return rotation; }
    
    // Target following
    void setTarget(const sf::Vector2f* target);
    void clearTarget() { targetPosition = nullptr; }
    void setFollowMode(CameraFollowMode mode) { followMode = mode; }
    void setFollowSpeed(float speed) { followSpeed = speed; }  // For smooth following
    
    // Deadzone (for deadzone follow mode)
    void setDeadzone(const sf::FloatRect& zone);
    sf::FloatRect getDeadzone() const { return deadzone; }
    
    // Screen shake (trauma-based system)
    void addTrauma(float amount);  // 0.0 to 1.0, trauma decays over time
    void setTrauma(float amount);
    float getTrauma() const { return trauma; }
    void setShakeIntensity(float intensity) { shakeIntensity = intensity; }
    void setTraumaDecay(float decay) { traumaDecay = decay; }
    
    // Bounds (optional - keeps camera within world boundaries)
    void setBounds(const sf::FloatRect& bounds);
    void clearBounds() { hasBounds = false; }
    bool isWithinBounds() const { return hasBounds; }
    
    // View conversion
    sf::View toSFMLView() const;
    
    // Coordinate conversion
    sf::Vector2f screenToWorld(const sf::Vector2f& screenPos, 
                              const sf::Vector2u& windowSize) const;
    sf::Vector2f worldToScreen(const sf::Vector2f& worldPos, 
                              const sf::Vector2u& windowSize) const;
    
    // Query
    sf::FloatRect getViewBounds() const;  // Get visible rectangle in world space
    bool isVisible(const sf::FloatRect& rect) const;  // Is rectangle visible?
    
private:
    // Core properties
    sf::Vector2f position;
    sf::Vector2f size;  // View size in world units
    float zoom;
    float rotation;  // Degrees
    
    // Smooth zoom
    bool isZooming;
    float targetZoom;
    float zoomDuration;
    float zoomProgress;
    
    // Following
    const sf::Vector2f* targetPosition;
    CameraFollowMode followMode;
    float followSpeed;  // For smooth following
    sf::FloatRect deadzone;  // For deadzone following
    
    // Screen shake
    float trauma;  // 0.0 to 1.0
    float traumaDecay;  // Units per second
    float shakeIntensity;  // Max shake offset in pixels
    mutable std::mt19937 rng;
    mutable std::uniform_real_distribution<float> randomDist;
    
    // Bounds
    bool hasBounds;
    sf::FloatRect bounds;
    
    // Helper methods
    void updateFollowing(float dt);
    void updateZoom(float dt);
    void updateShake(float dt);
    void applyBounds();
    sf::Vector2f calculateShakeOffset() const;
};

} // namespace Engine