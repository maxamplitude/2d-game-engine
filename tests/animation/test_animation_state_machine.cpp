#include <catch2/catch_test_macros.hpp>
#include "animation/animation_state_machine.h"
#include "animation/animation_controller.h"
#include "rendering/texture_atlas.h"

using namespace Engine;

// Test fixture
class StateMachineFixture {
public:
    StateMachineFixture() {
        setup_atlas();
    }
    
    TextureAtlas atlas;
    
private:
    void setup_atlas() {
        // Add frames
        atlas.add_frame(SpriteFrame("f0", glm::ivec4(0, 0, 16, 16)));
        atlas.add_frame(SpriteFrame("f1", glm::ivec4(16, 0, 16, 16)));
        atlas.add_frame(SpriteFrame("f2", glm::ivec4(32, 0, 16, 16)));
        
        // Animations
        atlas.add_animation(AnimationData("idle", {"f0", "f1"}, 0.1f, true));
        atlas.add_animation(AnimationData("walk", {"f0", "f1", "f2"}, 0.1f, true));
        atlas.add_animation(AnimationData("jump", {"f0", "f1"}, 0.1f, false));
        atlas.add_animation(AnimationData("attack", {"f0", "f1", "f2"}, 0.05f, false));
    }
};

TEST_CASE("AnimationStateMachine construction", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    
    SECTION("Valid construction") {
        AnimationStateMachine sm(&controller);
        REQUIRE(sm.get_current_state().empty());
    }
    
    SECTION("Null controller throws") {
        REQUIRE_THROWS_AS(AnimationStateMachine(nullptr), std::invalid_argument);
    }
}

TEST_CASE("AnimationStateMachine add states", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("idle", 0);
    sm.add_state("walk", 1);
    sm.add_state("jump", 2);
    
    auto state_names = sm.get_state_names();
    REQUIRE(state_names.size() == 3);
}

TEST_CASE("AnimationStateMachine transitions", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("idle", 0);
    sm.add_state("walk", 1);
    
    sm.transition_to("idle");
    REQUIRE(sm.get_current_state() == "idle");
    REQUIRE(controller.get_current_animation_name() == "idle");
    
    sm.transition_to("walk");
    REQUIRE(sm.get_current_state() == "walk");
    REQUIRE(controller.get_current_animation_name() == "walk");
}

TEST_CASE("AnimationStateMachine priority system", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("idle", 0);
    sm.add_state("walk", 1);
    sm.add_state("attack", 3);
    
    SECTION("Can transition to higher priority") {
        sm.transition_to("idle");
        sm.transition_to("attack");
        REQUIRE(sm.get_current_state() == "attack");
    }
    
    SECTION("Cannot transition to lower priority") {
        sm.transition_to("attack");
        sm.transition_to("idle");
        
        // Should stay in attack, idle is queued
        REQUIRE(sm.get_current_state() == "attack");
        REQUIRE(sm.has_pending_transition());
        REQUIRE(sm.get_pending_state() == "idle");
    }
    
    SECTION("Force transition overrides priority") {
        sm.transition_to("attack");
        sm.transition_to("idle", true);  // Force
        REQUIRE(sm.get_current_state() == "idle");
    }
}

TEST_CASE("AnimationStateMachine pending transitions", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("attack", 3);
    sm.add_state("idle", 0);
    
    sm.transition_to("attack");  // Non-looping animation
    sm.transition_to("idle");    // Can't interrupt, gets queued
    
    REQUIRE(sm.get_current_state() == "attack");
    REQUIRE(sm.has_pending_transition());
    
    // Update until attack finishes
    sm.update(0.15f);  // 3 frames * 0.05s
    
    // Should automatically transition to pending idle
    REQUIRE(sm.get_current_state() == "idle");
    REQUIRE_FALSE(sm.has_pending_transition());
}

TEST_CASE("AnimationStateMachine automatic transitions", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("idle", 0);
    sm.add_state("walk", 1);
    
    bool is_moving = false;
    
    SECTION("Immediate transition") {
        sm.add_transition("idle", "walk", TransitionCondition::Immediate,
                         [&]() { return is_moving; });
        
        sm.transition_to("idle");
        REQUIRE(sm.get_current_state() == "idle");
        
        is_moving = true;
        sm.update(0.01f);
        
        REQUIRE(sm.get_current_state() == "walk");
    }
    
    SECTION("OnFinish transition") {
        sm.add_state("jump", 2);
        sm.add_transition("jump", "idle", TransitionCondition::OnFinish);
        
        sm.transition_to("jump");
        REQUIRE(sm.get_current_state() == "jump");
        
        // Update until animation finishes
        sm.update(0.2f);
        
        REQUIRE(sm.get_current_state() == "idle");
    }
}

TEST_CASE("AnimationStateMachine predicate transitions", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("idle", 0);
    sm.add_state("walk", 1);
    sm.add_state("jump", 2);
    
    bool grounded = true;
    bool moving = false;
    
    // idle -> walk when moving and grounded
    sm.add_transition("idle", "walk", TransitionCondition::Immediate,
                     [&]() { return moving && grounded; });
    
    // walk -> idle when not moving
    sm.add_transition("walk", "idle", TransitionCondition::Immediate,
                     [&]() { return !moving; });
    
    // any -> jump when not grounded
    sm.add_transition("idle", "jump", TransitionCondition::Immediate,
                     [&]() { return !grounded; });
    sm.add_transition("walk", "jump", TransitionCondition::Immediate,
                     [&]() { return !grounded; });
    
    sm.transition_to("idle");
    
    SECTION("Walk when moving") {
        moving = true;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "walk");
    }
    
    SECTION("Jump when not grounded") {
        grounded = false;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "jump");
    }
    
    SECTION("Return to idle when stop moving") {
        moving = true;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "walk");
        
        moving = false;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "idle");
    }
}

TEST_CASE("AnimationStateMachine get state info", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("idle", 0);
    sm.add_state("attack", 3);
    
    const AnimationState* attack_state = sm.get_state("attack");
    REQUIRE(attack_state != nullptr);
    REQUIRE(attack_state->name == "attack");
    REQUIRE(attack_state->priority == 3);
    
    const AnimationState* invalid = sm.get_state("nonexistent");
    REQUIRE(invalid == nullptr);
}

TEST_CASE("AnimationStateMachine priority queries", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    sm.add_state("idle", 0);
    sm.add_state("walk", 1);
    sm.add_state("attack", 3);
    
    sm.transition_to("idle");
    REQUIRE(sm.get_current_priority() == 0);
    
    sm.transition_to("attack");
    REQUIRE(sm.get_current_priority() == 3);
}

TEST_CASE("AnimationStateMachine complex state graph", "[animation][statemachine]") {
    StateMachineFixture fixture;
    AnimationController controller(&fixture.atlas);
    AnimationStateMachine sm(&controller);
    
    // Setup platformer-style states
    sm.add_state("idle", 0);
    sm.add_state("walk", 1);
    sm.add_state("jump", 2);
    sm.add_state("attack", 3);
    
    bool is_moving = false;
    bool is_grounded = true;
    bool attack_pressed = false;
    
    // Transitions
    sm.add_transition("idle", "walk", TransitionCondition::Immediate,
                     [&]() { return is_moving && is_grounded; });
    sm.add_transition("walk", "idle", TransitionCondition::Immediate,
                     [&]() { return !is_moving && is_grounded; });
    sm.add_transition("idle", "jump", TransitionCondition::Immediate,
                     [&]() { return !is_grounded; });
    sm.add_transition("walk", "jump", TransitionCondition::Immediate,
                     [&]() { return !is_grounded; });
    sm.add_transition("jump", "idle", TransitionCondition::OnFinish,
                     [&]() { return is_grounded && !is_moving; });
    sm.add_transition("jump", "walk", TransitionCondition::OnFinish,
                     [&]() { return is_grounded && is_moving; });
    
    // Attack from any state
    sm.add_transition("idle", "attack", TransitionCondition::Immediate,
                     [&]() { return attack_pressed; });
    sm.add_transition("walk", "attack", TransitionCondition::Immediate,
                     [&]() { return attack_pressed; });
    
    sm.transition_to("idle");
    
    SECTION("Walking sequence") {
        is_moving = true;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "walk");
        
        is_moving = false;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "idle");
    }
    
    SECTION("Jump and land") {
        is_grounded = false;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "jump");
        
        is_grounded = true;
        sm.update(0.2f);  // Finish jump animation
        REQUIRE(sm.get_current_state() == "idle");
    }
    
    SECTION("Attack interrupt") {
        is_moving = true;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "walk");
        
        attack_pressed = true;
        sm.update(0.01f);
        REQUIRE(sm.get_current_state() == "attack");
    }
}