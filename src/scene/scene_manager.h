// src/scene/scene_manager.h
#pragma once
#include "scene.h"
#include <memory>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

namespace Engine {

/**
 * Transition types between scenes
 */
enum class SceneTransition {
    None,       // Instant cut
    Fade        // Fade out, switch, fade in
};

/**
 * Configuration for scene transitions
 */
struct TransitionConfig {
    SceneTransition type = SceneTransition::None;
    float duration = 0.5f;           // Seconds for fade
    sf::Color fadeColor = sf::Color::Black;
    
    TransitionConfig() = default;
    TransitionConfig(SceneTransition t, float d = 0.5f, sf::Color c = sf::Color::Black)
        : type(t), duration(d), fadeColor(c) {}
};

/**
 * Manages the scene stack and handles transitions.
 * 
 * Supports:
 * - changeScene(): Replace current scene
 * - pushScene(): Add scene on top (pause current)
 * - popScene(): Remove top scene (resume previous)
 * - Smooth transitions with fade effects
 * 
 * Scene Stack Example:
 * [GameplayScene] <- pushScene(PauseScene) -> [GameplayScene, PauseScene]
 * Only top scene receives updates, but all visible scenes render
 */
class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager() = default;
    
    // Scene transitions
    void changeScene(std::unique_ptr<Scene> newScene, 
                    const TransitionConfig& transition = TransitionConfig());
    void pushScene(std::unique_ptr<Scene> newScene,
                  const TransitionConfig& transition = TransitionConfig());
    void popScene(const TransitionConfig& transition = TransitionConfig());
    
    // Main loop integration
    void handleEvent(const sf::Event& event);
    void update(float dt);
    void render(sf::RenderWindow& window);
    
    // State queries
    bool hasActiveScene() const { return !sceneStack.empty(); }
    size_t getSceneCount() const { return sceneStack.size(); }
    bool isTransitioning() const { return transitionState != TransitionState::None; }
    
    // Template helper for type-safe scene creation
    template<typename T, typename... Args>
    std::unique_ptr<Scene> createScene(Args&&... args) {
        static_assert(std::is_base_of<Scene, T>::value, 
                     "T must derive from Scene");
        auto scene = std::make_unique<T>(std::forward<Args>(args)...);
        scene->manager = this;
        return scene;
    }
    
private:
    // Scene stack (top = active)
    std::vector<std::unique_ptr<Scene>> sceneStack;
    
    // Pending scene operations
    std::unique_ptr<Scene> pendingScene;
    std::function<void()> pendingOperation;
    
    // Transition state machine
    enum class TransitionState {
        None,
        FadingOut,
        FadingIn
    };
    
    TransitionState transitionState = TransitionState::None;
    TransitionConfig currentTransition;
    float transitionProgress = 0.0f;
    
    // Fade overlay
    sf::RectangleShape fadeOverlay;
    
    // Internal operations
    void executeChangeScene();
    void executePushScene();
    void executePopScene();
    void updateTransition(float dt);
    void renderFadeOverlay(sf::RenderWindow& window);
    void startTransition(const TransitionConfig& config, 
                        std::function<void()> operation);
};

} // namespace Engine