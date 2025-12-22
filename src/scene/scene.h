// src/scene/scene.h
#pragma once
#include <SFML/Graphics.hpp>

namespace Engine {

class SceneManager;

/**
 * Abstract base class for all game scenes (states).
 * 
 * Scenes represent distinct game states like:
 * - Main menu
 * - Gameplay
 * - Pause screen
 * - Game over screen
 * 
 * Lifecycle:
 * 1. onEnter() - Called when scene becomes active
 * 2. update(dt) - Called every frame while active
 * 3. render(window) - Called every frame for drawing
 * 4. onExit() - Called when scene is being removed
 * 
 * Optional:
 * - handleEvent() - Process SFML events
 * - onPause()/onResume() - For scene stacking
 */
class Scene {
public:
    SceneManager* getManager() const { return manager; }

    virtual ~Scene() = default;
    
    // Lifecycle hooks (all optional to override)
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onPause() {}   // When another scene is pushed on top
    virtual void onResume() {}  // When scene on top is popped
    
    // Main loop (must override these)
    virtual void handleEvent(const sf::Event& event) {}
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    
protected:
    // Scenes can access the manager to request transitions
    Scene() = default;
    SceneManager* manager = nullptr;
    
    friend class SceneManager;
};

} // namespace Engine