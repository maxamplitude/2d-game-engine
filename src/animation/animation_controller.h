// src/Animation/AnimationController.h
#pragma once
#include "rendering/texture_atlas.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace Engine {

class AnimationController {
public:
    AnimationController(TextureAtlas* atlas);
    
    // Playback control
    void play(const std::string& animationName, bool restart = false);
    void stop();
    void pause();
    void resume();
    
    // Update (call every frame)
    void update(float dt);
    
    // Get current sprite to render
    sf::Sprite getCurrentSprite() const;
    const SpriteFrame* getCurrentFrame() const;
    
    // Query state
    bool isPlaying() const { return playing; }
    bool isFinished() const { return finished; }
    std::string getCurrentAnimationName() const { return currentAnimName; }
    int getCurrentFrameIndex() const { return currentFrameIndex; }
    
    // Callbacks (optional)
    void setOnAnimationEnd(std::function<void()> callback) { 
        onAnimationEnd = callback; 
    }
    void setOnFrameChange(std::function<void(int)> callback) {
        onFrameChange = callback;
    }
    
private:
    TextureAtlas* atlas;
    const AnimationData* currentAnim = nullptr;
    std::string currentAnimName;
    
    int currentFrameIndex = 0;
    float currentFrameTime = 0.0f;
    bool playing = false;
    bool finished = false;
    bool paused = false;
    
    // Callbacks
    std::function<void()> onAnimationEnd;
    std::function<void(int)> onFrameChange;
    
    float getFrameDuration(int frameIndex) const;
};

} // namespace Engine