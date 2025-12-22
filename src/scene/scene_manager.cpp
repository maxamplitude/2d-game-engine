// src/scene/scene_manager.cpp
#include "scene_manager.h"
#include <iostream>

namespace Engine {

void SceneManager::changeScene(std::unique_ptr<Scene> newScene, 
                              const TransitionConfig& transition) {
    if (!newScene) {
        std::cerr << "Warning: Attempted to change to null scene" << std::endl;
        return;
    }
    
    pendingScene = std::move(newScene);
    
    if (transition.type == SceneTransition::None) {
        // Immediate transition
        executeChangeScene();
    } else {
        // Deferred transition with fade
        startTransition(transition, [this]() { executeChangeScene(); });
    }
}

void SceneManager::pushScene(std::unique_ptr<Scene> newScene,
                            const TransitionConfig& transition) {
    if (!newScene) {
        std::cerr << "Warning: Attempted to push null scene" << std::endl;
        return;
    }
    
    pendingScene = std::move(newScene);
    
    if (transition.type == SceneTransition::None) {
        executePushScene();
    } else {
        startTransition(transition, [this]() { executePushScene(); });
    }
}

void SceneManager::popScene(const TransitionConfig& transition) {
    if (sceneStack.empty()) {
        std::cerr << "Warning: Attempted to pop from empty scene stack" << std::endl;
        return;
    }
    
    if (transition.type == SceneTransition::None) {
        executePopScene();
    } else {
        startTransition(transition, [this]() { executePopScene(); });
    }
}

void SceneManager::handleEvent(const sf::Event& event) {
    // Only top scene receives events
    if (!sceneStack.empty() && transitionState == TransitionState::None) {
        sceneStack.back()->handleEvent(event);
    }
}

void SceneManager::update(float dt) {
    if (transitionState != TransitionState::None) {
        updateTransition(dt);
        return;
    }
    
    // Only update top scene
    if (!sceneStack.empty()) {
        sceneStack.back()->update(dt);
    }
}

void SceneManager::render(sf::RenderWindow& window) {
    // Render all scenes (bottom to top) for transparency effects
    // Note: Most games will only render the top scene, but this allows
    // for effects like dimmed background during pause screens
    for (auto& scene : sceneStack) {
        scene->render(window);
    }
    
    // Render fade overlay if transitioning
    if (transitionState != TransitionState::None) {
        renderFadeOverlay(window);
    }
}

// ============================================================================
// Private: Scene Operations
// ============================================================================

void SceneManager::executeChangeScene() {
    // Exit current scene
    if (!sceneStack.empty()) {
        sceneStack.back()->onExit();
        sceneStack.clear();
    }
    
    // Enter new scene
    if (pendingScene) {
        pendingScene->manager = this;
        pendingScene->onEnter();
        sceneStack.push_back(std::move(pendingScene));
    }
}

void SceneManager::executePushScene() {
    // Pause current scene
    if (!sceneStack.empty()) {
        sceneStack.back()->onPause();
    }
    
    // Enter new scene
    if (pendingScene) {
        pendingScene->manager = this;
        pendingScene->onEnter();
        sceneStack.push_back(std::move(pendingScene));
    }
}

void SceneManager::executePopScene() {
    if (sceneStack.empty()) return;
    
    // Exit top scene
    sceneStack.back()->onExit();
    sceneStack.pop_back();
    
    // Resume previous scene
    if (!sceneStack.empty()) {
        sceneStack.back()->onResume();
    }
}

// ============================================================================
// Private: Transition Management
// ============================================================================

void SceneManager::startTransition(const TransitionConfig& config, 
                                   std::function<void()> operation) {
    currentTransition = config;
    pendingOperation = operation;
    transitionState = TransitionState::FadingOut;
    transitionProgress = 0.0f;
    
    // Initialize fade overlay (will be sized on first render)
    fadeOverlay.setFillColor(config.fadeColor);
}

void SceneManager::updateTransition(float dt) {
    transitionProgress += dt;
    
    if (transitionState == TransitionState::FadingOut) {
        // Fade out phase
        if (transitionProgress >= currentTransition.duration) {
            // Execute the pending scene operation
            if (pendingOperation) {
                pendingOperation();
                pendingOperation = nullptr;
            }
            
            // Start fade in
            transitionState = TransitionState::FadingIn;
            transitionProgress = 0.0f;
        }
    } 
    else if (transitionState == TransitionState::FadingIn) {
        // Fade in phase
        if (transitionProgress >= currentTransition.duration) {
            // Transition complete
            transitionState = TransitionState::None;
            transitionProgress = 0.0f;
        }
    }
}

void SceneManager::renderFadeOverlay(sf::RenderWindow& window) {
    // Ensure overlay matches window size
    auto windowSize = window.getSize();
    fadeOverlay.setSize(sf::Vector2f(
        static_cast<float>(windowSize.x),
        static_cast<float>(windowSize.y)
    ));
    
    // Calculate alpha based on transition phase
    float alpha = 0.0f;
    float normalizedProgress = transitionProgress / currentTransition.duration;
    normalizedProgress = std::min(normalizedProgress, 1.0f);
    
    if (transitionState == TransitionState::FadingOut) {
        // 0 -> 255 (fade to solid color)
        alpha = normalizedProgress * 255.0f;
    } else if (transitionState == TransitionState::FadingIn) {
        // 255 -> 0 (fade from solid color)
        alpha = (1.0f - normalizedProgress) * 255.0f;
    }
    
    sf::Color overlayColor = currentTransition.fadeColor;
    overlayColor.a = static_cast<sf::Uint8>(alpha);
    fadeOverlay.setFillColor(overlayColor);
    
    window.draw(fadeOverlay);
}

} // namespace Engine