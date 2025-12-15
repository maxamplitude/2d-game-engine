// src/Animation/AnimationController.cpp
#include "animation_controller.h"

namespace Engine {

AnimationController::AnimationController(TextureAtlas* atlas) 
    : atlas(atlas) {}

void AnimationController::play(const std::string& animationName, bool restart) {
    const AnimationData* anim = atlas->getAnimation(animationName);
    if (!anim) return;
    
    // Already playing this animation?
    if (currentAnim == anim && !restart) {
        return;
    }
    
    currentAnim = anim;
    currentAnimName = animationName;
    currentFrameIndex = 0;
    currentFrameTime = 0.0f;
    playing = true;
    finished = false;
    paused = false;
}

void AnimationController::stop() {
    playing = false;
    finished = true;
    currentFrameIndex = 0;
    currentFrameTime = 0.0f;
}

void AnimationController::pause() {
    paused = true;
}

void AnimationController::resume() {
    paused = false;
}

void AnimationController::update(float dt) {
    if (!playing || paused || !currentAnim) return;
    
    currentFrameTime += dt;
    float frameDuration = getFrameDuration(currentFrameIndex);
    
    // Advance frame
    while (currentFrameTime >= frameDuration) {
        currentFrameTime -= frameDuration;
        
        int previousFrame = currentFrameIndex;
        currentFrameIndex++;
        
        // End of animation?
        if (currentFrameIndex >= static_cast<int>(currentAnim->frameNames.size())) {
            if (currentAnim->loop) {
                currentFrameIndex = 0;
            } else {
                currentFrameIndex = static_cast<int>(currentAnim->frameNames.size()) - 1;
                playing = false;
                finished = true;
                
                if (onAnimationEnd) {
                    onAnimationEnd();
                }
                return;
            }
        }
        
        // Frame changed callback
        if (onFrameChange && currentFrameIndex != previousFrame) {
            onFrameChange(currentFrameIndex);
        }
        
        frameDuration = getFrameDuration(currentFrameIndex);
    }
}

sf::Sprite AnimationController::getCurrentSprite() const {
    const SpriteFrame* frame = getCurrentFrame();
    if (!frame) return sf::Sprite();
    
    sf::Sprite sprite(atlas->getTexture(), frame->rect);
    sprite.setOrigin(frame->origin);
    return sprite;
}

const SpriteFrame* AnimationController::getCurrentFrame() const {
    if (!currentAnim || currentFrameIndex >= static_cast<int>(currentAnim->frameNames.size())) {
        return nullptr;
    }
    
    const std::string& frameName = currentAnim->frameNames[currentFrameIndex];
    return atlas->getFrame(frameName);
}

float AnimationController::getFrameDuration(int frameIndex) const {
    if (!currentAnim) return 0.0f;
    
    // Variable frame durations?
    if (!currentAnim->frameDurations.empty() && 
        frameIndex < static_cast<int>(currentAnim->frameDurations.size())) {
        return currentAnim->frameDurations[frameIndex];
    }
    
    // Default duration
    return currentAnim->frameDuration;
}

} // namespace Engine