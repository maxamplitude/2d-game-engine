// tests/Animation/test_AnimationController.cpp
#include <catch2/catch_test_macros.hpp>
#include "animation/animation_controller.h"
#include "rendering/texture_atlas.h"

using namespace Engine;

TEST_CASE("Animation plays frames in sequence", "[animation]") {
    // Setup mock atlas
    TextureAtlas atlas;
    // ... load test animation data
    
    AnimationController controller(&atlas);
    controller.play("test_walk");
    
    REQUIRE(controller.isPlaying());
    REQUIRE(controller.getCurrentFrameIndex() == 0);
    
    // Advance by one frame duration
    controller.update(0.1f);
    REQUIRE(controller.getCurrentFrameIndex() == 1);
    
    controller.update(0.1f);
    REQUIRE(controller.getCurrentFrameIndex() == 2);
}

TEST_CASE("Looping animation repeats", "[animation]") {
    TextureAtlas atlas;
    AnimationController controller(&atlas);
    
    controller.play("test_loop");  // 4 frames, loops
    
    // Advance through all frames
    for (int i = 0; i < 4; ++i) {
        controller.update(0.1f);
    }
    
    // Should loop back to frame 0
    REQUIRE(controller.getCurrentFrameIndex() == 0);
    REQUIRE(controller.isPlaying());
}

TEST_CASE("Non-looping animation stops", "[animation]") {
    TextureAtlas atlas;
    AnimationController controller(&atlas);
    
    bool callbackFired = false;
    controller.setOnAnimationEnd([&]() { callbackFired = true; });
    
    controller.play("test_once");  // 3 frames, no loop
    
    for (int i = 0; i < 3; ++i) {
        controller.update(0.1f);
    }
    
    REQUIRE(controller.isFinished());
    REQUIRE_FALSE(controller.isPlaying());
    REQUIRE(callbackFired);
}