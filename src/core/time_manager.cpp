#include "time_manager.h"

namespace Engine {

void TimeManager::update() {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> diff = now - lastTick;
    deltaTime = diff.count();

    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }

    totalTime += deltaTime;
    frameCount++;
    lastTick = now;
}

void TimeManager::reset() {
    lastTick = std::chrono::steady_clock::now();
    deltaTime = 0.0f;
    totalTime = 0.0f;
    frameCount = 0;
}

float TimeManager::getFPS() const {
    return deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
}

} // namespace Engine