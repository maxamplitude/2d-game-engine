#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>
#include "core/time_manager.h"

using namespace Engine;

TEST_CASE("TimeManager initial state", "[time][core]") {
    TimeManager timer;
    
    REQUIRE(timer.getTotalTime() == 0.0f);
    REQUIRE(timer.getFrameCount() == 0);
    REQUIRE(timer.getDeltaTime() == 0.0f);
}

TEST_CASE("TimeManager tracks delta time", "[time][core]") {
    TimeManager timer;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    timer.update();
    
    REQUIRE(timer.getDeltaTime() > 0.0f);
    REQUIRE(timer.getDeltaTime() < 0.1f);  // Should be clamped
    REQUIRE(timer.getFrameCount() == 1);
}

TEST_CASE("TimeManager accumulates total time", "[time][core]") {
    TimeManager timer;
    
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.update();
    }
    
    REQUIRE(timer.getFrameCount() == 5);
    REQUIRE(timer.getTotalTime() > 0.0f);
}

TEST_CASE("TimeManager calculates FPS", "[time][core]") {
    TimeManager timer;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    timer.update();
    
    float fps = timer.getFPS();
    REQUIRE(fps > 0.0f);
    REQUIRE(fps < 200.0f);  // Sanity check
}

TEST_CASE("TimeManager reset", "[time][core]") {
    TimeManager timer;
    
    timer.update();
    timer.update();
    
    REQUIRE(timer.getFrameCount() == 2);
    
    timer.reset();
    
    REQUIRE(timer.getFrameCount() == 0);
    REQUIRE(timer.getTotalTime() == 0.0f);
}