#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <glm/glm.hpp>
#include <stdexcept>
#include "animation/animation_controller.h"
#include "rendering/texture_atlas.h"

using namespace Engine;

// Test fixture - creates a mock atlas with test animations
class AnimationControllerFixture {
public:
    AnimationControllerFixture() {
        setup_atlas();
    }
    
    TextureAtlas atlas;
    
private:
    void setup_atlas() {
        // Add test frames
        atlas.add_frame(SpriteFrame("frame_0", glm::ivec4(0, 0, 16, 16)));
        atlas.add_frame(SpriteFrame("frame_1", glm::ivec4(16, 0, 16, 16)));
        atlas.add_frame(SpriteFrame("frame_2", glm::ivec4(32, 0, 16, 16)));
        atlas.add_frame(SpriteFrame("frame_3", glm::ivec4(48, 0, 16, 16)));
        
        // Looping animation (4 frames)
        AnimationData loop_anim("loop_anim", 
            {"frame_0", "frame_1", "frame_2", "frame_3"}, 
            0.1f, true);
        atlas.add_animation(loop_anim);
        
        // Non-looping animation (3 frames)
        AnimationData once_anim("once_anim",
            {"frame_0", "frame_1", "frame_2"},
            0.1f, false);
        atlas.add_animation(once_anim);
        
        // Variable frame durations
        AnimationData var_anim("var_anim",
            {"frame_0", "frame_1", "frame_2"},
            0.1f, true);
        var_anim.frame_durations = {0.05f, 0.15f, 0.2f};
        atlas.add_animation(var_anim);
    }
};

TEST_CASE("AnimationController construction", "[animation][controller]") {
    AnimationControllerFixture fixture;
    
    SECTION("Valid construction") {
        AnimationController controller(&fixture.atlas);
        REQUIRE_FALSE(controller.is_playing());
        REQUIRE_FALSE(controller.has_animation());
    }
    
    SECTION("Null atlas throws") {
        REQUIRE_THROWS_AS(AnimationController(nullptr), std::invalid_argument);
    }
}

TEST_CASE("AnimationController plays animations", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    
    REQUIRE(controller.is_playing());
    REQUIRE(controller.has_animation());
    REQUIRE(controller.get_current_animation_name() == "loop_anim");
    REQUIRE(controller.get_current_frame_index() == 0);
    REQUIRE_FALSE(controller.is_finished());
}

TEST_CASE("AnimationController advances frames", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    
    SECTION("Single frame advance") {
        controller.update(0.1f);  // One frame duration
        REQUIRE(controller.get_current_frame_index() == 1);
    }
    
    SECTION("Multiple frame advance") {
        controller.update(0.3f);  // Three frames
        REQUIRE(controller.get_current_frame_index() == 3);
    }
    
    SECTION("Partial frame advance") {
        controller.update(0.05f);  // Half a frame
        REQUIRE(controller.get_current_frame_index() == 0);
        
        controller.update(0.05f);  // Complete the frame
        REQUIRE(controller.get_current_frame_index() == 1);
    }
}

TEST_CASE("AnimationController loops animations", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");  // 4 frames, 0.1s each
    
    // Advance past end
    controller.update(0.4f);  // Complete all 4 frames
    
    REQUIRE(controller.get_current_frame_index() == 0);  // Looped back
    REQUIRE(controller.is_playing());
    REQUIRE_FALSE(controller.is_finished());
}

TEST_CASE("AnimationController stops non-looping animations", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("once_anim");  // 3 frames, no loop
    
    // Advance to end
    controller.update(0.3f);
    
    REQUIRE(controller.get_current_frame_index() == 2);  // Last frame
    REQUIRE_FALSE(controller.is_playing());
    REQUIRE(controller.is_finished());
}

TEST_CASE("AnimationController pause and resume", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    
    SECTION("Pause stops frame advancement") {
        controller.pause();
        REQUIRE(controller.is_paused());
        
        controller.update(0.5f);  // Try to advance
        REQUIRE(controller.get_current_frame_index() == 0);  // Still frame 0
    }
    
    SECTION("Resume continues playback") {
        controller.pause();
        controller.resume();
        REQUIRE_FALSE(controller.is_paused());
        
        controller.update(0.1f);
        REQUIRE(controller.get_current_frame_index() == 1);
    }
}

TEST_CASE("AnimationController stop", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    controller.update(0.2f);  // Advance to frame 2
    
    controller.stop();
    
    REQUIRE_FALSE(controller.is_playing());
    REQUIRE(controller.is_finished());
    REQUIRE(controller.get_current_frame_index() == 0);  // Reset to frame 0
}

TEST_CASE("AnimationController reset", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    controller.update(0.2f);
    
    REQUIRE(controller.get_current_frame_index() == 2);
    
    controller.reset();
    
    REQUIRE(controller.get_current_frame_index() == 0);
    REQUIRE(controller.is_playing());  // Still playing
    REQUIRE_FALSE(controller.is_finished());
}

TEST_CASE("AnimationController playback speed", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    
    SECTION("Normal speed") {
        controller.set_speed(1.0f);
        controller.update(0.1f);
        REQUIRE(controller.get_current_frame_index() == 1);
    }
    
    SECTION("Double speed") {
        controller.set_speed(2.0f);
        controller.update(0.1f);  // 0.1 * 2 = 0.2 effective time
        REQUIRE(controller.get_current_frame_index() == 2);
    }
    
    SECTION("Half speed") {
        controller.set_speed(0.5f);
        controller.update(0.2f);  // 0.2 * 0.5 = 0.1 effective time
        REQUIRE(controller.get_current_frame_index() == 1);
    }
}

TEST_CASE("AnimationController variable frame durations", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("var_anim");  // Durations: 0.05, 0.15, 0.2
    
    SECTION("First frame (0.05s)") {
        controller.update(0.05f);
        REQUIRE(controller.get_current_frame_index() == 1);
    }
    
    SECTION("Second frame (0.15s)") {
        controller.update(0.05f);  // Frame 1
        controller.update(0.15f);  // Frame 2
        REQUIRE(controller.get_current_frame_index() == 2);
    }
    
    SECTION("Third frame (0.2s)") {
        controller.update(0.4f);  // 0.05 + 0.15 + 0.2
        REQUIRE(controller.get_current_frame_index() == 0);  // Looped
    }
}

TEST_CASE("AnimationController callbacks", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    SECTION("on_frame_change callback") {
        int callback_count = 0;
        int last_frame = -1;
        
        controller.set_on_frame_change([&](int frame) {
            callback_count++;
            last_frame = frame;
        });
        
        controller.play("loop_anim");
        controller.update(0.2f);  // Advance 2 frames
        
        REQUIRE(callback_count == 2);
        REQUIRE(last_frame == 2);
    }
    
    SECTION("on_animation_end callback") {
        bool end_called = false;
        
        controller.set_on_animation_end([&]() {
            end_called = true;
        });
        
        controller.play("once_anim");
        controller.update(0.3f);  // Complete animation
        
        REQUIRE(end_called);
    }
    
    SECTION("on_animation_loop callback") {
        int loop_count = 0;
        
        controller.set_on_animation_loop([&]() {
            loop_count++;
        });
        
        controller.play("loop_anim");
        controller.update(0.8f);  // Two complete loops
        
        REQUIRE(loop_count == 2);
    }
}

TEST_CASE("AnimationController play with restart", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    controller.update(0.2f);  // Advance to frame 2
    
    SECTION("Play same animation without restart") {
        controller.play("loop_anim", false);
        REQUIRE(controller.get_current_frame_index() == 2);  // Unchanged
    }
    
    SECTION("Play same animation with restart") {
        controller.play("loop_anim", true);
        REQUIRE(controller.get_current_frame_index() == 0);  // Reset
    }
    
    SECTION("Play different animation always restarts") {
        controller.play("once_anim", false);
        REQUIRE(controller.get_current_frame_index() == 0);
    }
}

TEST_CASE("AnimationController get current frame", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    
    const SpriteFrame* frame = controller.get_current_frame();
    
    REQUIRE(frame != nullptr);
    REQUIRE(frame->name == "frame_0");
    REQUIRE(frame->pixel_rect.x == 0);
    REQUIRE(frame->pixel_rect.z == 16);
}

TEST_CASE("AnimationController progress", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");  // 4 frames
    
    SECTION("Start") {
        REQUIRE_THAT(controller.get_progress(), 
                    Catch::Matchers::WithinRel(0.0f, 0.01f));
    }
    
    SECTION("Middle") {
        controller.update(0.1f);  // Frame 1
        REQUIRE_THAT(controller.get_progress(), 
                    Catch::Matchers::WithinRel(0.333f, 0.01f));
    }
    
    SECTION("End") {
        controller.update(0.3f);  // Frame 3 (last)
        REQUIRE_THAT(controller.get_progress(), 
                    Catch::Matchers::WithinRel(1.0f, 0.01f));
    }
}

TEST_CASE("AnimationController invalid animation name", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    controller.play("loop_anim");
    REQUIRE(controller.is_playing());
    
    // Try to play non-existent animation
    controller.play("does_not_exist");
    
    // Should stop current playback
    REQUIRE_FALSE(controller.is_playing());
    REQUIRE_FALSE(controller.has_animation());
}

TEST_CASE("AnimationController clear callbacks", "[animation][controller]") {
    AnimationControllerFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    bool called = false;
    controller.set_on_animation_end([&]() { called = true; });
    
    controller.clear_callbacks();
    
    controller.play("once_anim");
    controller.update(0.3f);
    
    REQUIRE_FALSE(called);  // Callback was cleared
}

