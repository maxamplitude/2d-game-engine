#include "time_manager.h"

namespace Engine {

void TimeManager::update() {
    deltaTime = clock.restart().asSeconds();
    
    // Clamp delta time to prevent huge jumps (e.g., when debugging)
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }
    
    totalTime += deltaTime;
    frameCount++;
}

void TimeManager::reset() {
    clock.restart();
    deltaTime = 0.0f;
    totalTime = 0.0f;
    frameCount = 0;
}

float TimeManager::getFPS() const {
    return deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
}

} // namespace Engine