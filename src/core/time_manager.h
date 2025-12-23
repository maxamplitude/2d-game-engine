#pragma once
#include <chrono>

namespace Engine {

class TimeManager {
public:
    TimeManager() = default;
    
    void update();
    void reset();
    
    float getDeltaTime() const { return deltaTime; }
    float getTotalTime() const { return totalTime; }
    int getFrameCount() const { return frameCount; }
    float getFPS() const;
    
private:
    std::chrono::steady_clock::time_point lastTick{std::chrono::steady_clock::now()};
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    int frameCount = 0;
};

} // namespace Engine