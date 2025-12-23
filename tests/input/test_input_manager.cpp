#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "input/input_manager.h"
#include <GLFW/glfw3.h>

using namespace Engine;

TEST_CASE("InputManager initial state", "[input]") {
    InputManager input;
    
    SECTION("All keys start as Up") {
        REQUIRE(input.getKeyState(GLFW_KEY_A) == InputState::Up);
        REQUIRE_FALSE(input.isKeyDown(GLFW_KEY_SPACE));
        REQUIRE_FALSE(input.isKeyPressed(GLFW_KEY_SPACE));
    }
    
    SECTION("Has default action mappings") {
        REQUIRE(input.hasKeyBinding("jump"));
        REQUIRE(input.hasKeyBinding("move_left"));
        REQUIRE(input.hasKeyBinding("move_right"));
    }
}

TEST_CASE("InputManager tracks key press", "[input]") {
    InputManager input;
    
    input.beginFrame();
    input.handleKeyPressed(GLFW_KEY_SPACE);
    
    SECTION("Key is just pressed on first frame") {
        REQUIRE(input.getKeyState(GLFW_KEY_SPACE) == InputState::JustPressed);
        REQUIRE(input.isKeyPressed(GLFW_KEY_SPACE));
        REQUIRE(input.isKeyDown(GLFW_KEY_SPACE));
        REQUIRE_FALSE(input.isKeyReleased(GLFW_KEY_SPACE));
    }
    
    SECTION("Key becomes held on second frame") {
        input.update(0.016f);
        input.beginFrame();
        
        REQUIRE(input.getKeyState(GLFW_KEY_SPACE) == InputState::Held);
        REQUIRE_FALSE(input.isKeyPressed(GLFW_KEY_SPACE));
        REQUIRE(input.isKeyDown(GLFW_KEY_SPACE));
    }
}

TEST_CASE("InputManager tracks key release", "[input]") {
    InputManager input;
    
    // Press key
    input.beginFrame();
    input.handleKeyPressed(GLFW_KEY_SPACE);
    input.update(0.016f);
    
    // Hold for one frame
    input.beginFrame();
    input.update(0.016f);
    
    // Release key
    input.beginFrame();
    input.handleKeyReleased(GLFW_KEY_SPACE);
    
    SECTION("Key is just released") {
        REQUIRE(input.getKeyState(GLFW_KEY_SPACE) == InputState::JustReleased);
        REQUIRE(input.isKeyReleased(GLFW_KEY_SPACE));
        REQUIRE_FALSE(input.isKeyDown(GLFW_KEY_SPACE));
        REQUIRE_FALSE(input.isKeyPressed(GLFW_KEY_SPACE));
    }
    
    SECTION("Key becomes Up on next frame") {
        input.update(0.016f);
        input.beginFrame();
        
        REQUIRE(input.getKeyState(GLFW_KEY_SPACE) == InputState::Up);
        REQUIRE_FALSE(input.isKeyDown(GLFW_KEY_SPACE));
    }
}

TEST_CASE("InputManager action mapping", "[input]") {
    InputManager input;
    
    SECTION("Maps action to key") {
        input.mapAction("test_action", GLFW_KEY_Q);
        REQUIRE(input.hasKeyBinding("test_action"));
        REQUIRE(input.getKeyBinding("test_action") == GLFW_KEY_Q);
    }
    
    SECTION("Maps action to mouse button") {
        input.mapActionMouse("test_action", GLFW_MOUSE_BUTTON_RIGHT);
        REQUIRE(input.hasMouseBinding("test_action"));
        REQUIRE(input.getMouseBinding("test_action") == GLFW_MOUSE_BUTTON_RIGHT);
    }
    
    SECTION("Remapping replaces existing binding") {
        input.mapAction("jump", GLFW_KEY_SPACE);
        input.mapAction("jump", GLFW_KEY_W);
        
        REQUIRE(input.getKeyBinding("jump") == GLFW_KEY_W);
    }
    
    SECTION("Can unmap action") {
        input.mapAction("test", GLFW_KEY_T);
        input.unmapAction("test");
        
        REQUIRE_FALSE(input.hasKeyBinding("test"));
    }
}

TEST_CASE("InputManager action queries", "[input]") {
    InputManager input;
    input.mapAction("test_action", GLFW_KEY_T);
    
    SECTION("Action is active when key is down") {
        input.beginFrame();
        input.handleKeyPressed(GLFW_KEY_T);
        
        REQUIRE(input.isActionActive("test_action"));
        REQUIRE(input.isActionPressed("test_action"));
    }
    
    SECTION("Action pressed only on first frame") {
        input.beginFrame();
        input.handleKeyPressed(GLFW_KEY_T);
        REQUIRE(input.isActionPressed("test_action"));
        
        input.update(0.016f);
        input.beginFrame();
        REQUIRE_FALSE(input.isActionPressed("test_action"));
        REQUIRE(input.isActionActive("test_action"));  // Still held
    }
    
    SECTION("Action released detection") {
        // Press
        input.beginFrame();
        input.handleKeyPressed(GLFW_KEY_T);
        input.update(0.016f);
        
        // Release
        input.beginFrame();
        input.handleKeyReleased(GLFW_KEY_T);
        
        REQUIRE(input.isActionReleased("test_action"));
        REQUIRE_FALSE(input.isActionActive("test_action"));
    }
}

TEST_CASE("InputManager input buffering", "[input]") {
    InputManager input;
    
    SECTION("Buffers action for specified time") {
        input.bufferAction("jump", 0.15f);
        
        REQUIRE(input.consumeBufferedAction("jump"));
        REQUIRE_FALSE(input.consumeBufferedAction("jump"));  // Already consumed
    }
    
    SECTION("Buffer expires after time") {
        input.bufferAction("jump", 0.1f);
        
        input.beginFrame();
        input.update(0.05f);  // 50ms
        REQUIRE(input.consumeBufferedAction("jump"));  // Still valid
        
        input.bufferAction("jump", 0.1f);
        input.beginFrame();
        input.update(0.15f);  // 150ms - expired
        REQUIRE_FALSE(input.consumeBufferedAction("jump"));
    }
    
    SECTION("Multiple buffered actions") {
        input.bufferAction("jump", 0.1f);
        input.bufferAction("attack", 0.2f);
        
        REQUIRE(input.consumeBufferedAction("jump"));
        REQUIRE(input.consumeBufferedAction("attack"));
    }
    
    SECTION("Can clear all buffers") {
        input.bufferAction("jump", 0.5f);
        input.bufferAction("attack", 0.5f);
        
        input.clearBuffer();
        
        REQUIRE_FALSE(input.consumeBufferedAction("jump"));
        REQUIRE_FALSE(input.consumeBufferedAction("attack"));
    }
    
    SECTION("Buffering same action extends time") {
        input.bufferAction("jump", 0.1f);
        input.beginFrame();
        input.update(0.05f);
        
        // Re-buffer resets the timer
        input.bufferAction("jump", 0.1f);
        input.beginFrame();
        input.update(0.08f);
        
        // Should still be valid (0.08s < 0.1s from second buffer)
        REQUIRE(input.consumeBufferedAction("jump"));
    }
}

TEST_CASE("InputManager mouse button support", "[input]") {
    InputManager input;
    
    SECTION("Tracks mouse button press") {
        input.beginFrame();
        input.handleMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        
        REQUIRE(input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT));
        REQUIRE(input.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT));
        REQUIRE(input.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == InputState::JustPressed);
    }
    
    SECTION("Mouse button becomes held") {
        input.beginFrame();
        input.handleMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        input.update(0.016f);
        
        input.beginFrame();
        
        REQUIRE(input.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == InputState::Held);
        REQUIRE_FALSE(input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT));
        REQUIRE(input.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT));
    }
    
    SECTION("Tracks mouse button release") {
        // Press
        input.beginFrame();
        input.handleMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        input.update(0.016f);
        
        // Release
        input.beginFrame();
        input.handleMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT);
        
        REQUIRE(input.isMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT));
        REQUIRE_FALSE(input.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT));
    }
}

TEST_CASE("InputManager action callbacks", "[input]") {
    InputManager input;
    input.mapAction("test_action", GLFW_KEY_T);
    
    bool callbackFired = false;
    input.setActionPressedCallback("test_action", [&]() {
        callbackFired = true;
    });
    
    SECTION("Callback fires when action pressed") {
        input.beginFrame();
        input.handleKeyPressed(GLFW_KEY_T);
        input.update(0.016f);  // Triggers callbacks
        
        REQUIRE(callbackFired);
    }
    
    SECTION("Callback doesn't fire when action held") {
        // Press
        input.beginFrame();
        input.handleKeyPressed(GLFW_KEY_T);
        input.update(0.016f);
        callbackFired = false;
        
        // Hold (next frame)
        input.beginFrame();
        input.update(0.016f);
        
        REQUIRE_FALSE(callbackFired);  // Only fires on press, not hold
    }
    
    SECTION("Can clear callbacks") {
        input.clearActionCallbacks();
        
        input.beginFrame();
        input.handleKeyPressed(GLFW_KEY_T);
        input.update(0.016f);
        
        REQUIRE_FALSE(callbackFired);
    }
}

TEST_CASE("InputManager multiple keys for same action", "[input]") {
    InputManager input;
    
    // Note: Current design only supports one binding per action
    // If you need multiple bindings, you'd need to track multiple keys per action
    
    input.mapAction("move_right", GLFW_KEY_D);
    input.mapAction("move_right_alt", GLFW_KEY_RIGHT);
    
    input.beginFrame();
    input.handleKeyPressed(GLFW_KEY_D);
    
    REQUIRE(input.isActionActive("move_right"));
    REQUIRE_FALSE(input.isActionActive("move_right_alt"));
}

TEST_CASE("InputManager prevents double buffering active actions", "[input]") {
    InputManager input;
    input.mapAction("jump", GLFW_KEY_SPACE);
    
    input.beginFrame();
    input.handleKeyPressed(GLFW_KEY_SPACE);
    
    // Try to buffer while action is active
    input.bufferAction("jump", 0.1f);
    
    // Release and try to consume buffer
    input.update(0.016f);
    input.beginFrame();
    input.handleKeyReleased(GLFW_KEY_SPACE);
    input.update(0.016f);
    
    // Buffer shouldn't exist because action was active when buffered
    REQUIRE_FALSE(input.consumeBufferedAction("jump"));
}

TEST_CASE("InputManager clear all mappings", "[input]") {
    InputManager input;
    
    input.mapAction("action1", GLFW_KEY_A);
    input.mapAction("action2", GLFW_KEY_B);
    input.mapActionMouse("action3", GLFW_MOUSE_BUTTON_LEFT);
    
    input.clearAllMappings();
    
    REQUIRE_FALSE(input.hasKeyBinding("action1"));
    REQUIRE_FALSE(input.hasKeyBinding("action2"));
    REQUIRE_FALSE(input.hasMouseBinding("action3"));
}

TEST_CASE("InputManager handles rapid press-release", "[input]") {
    InputManager input;
    
    // Press and release in same frame (before update)
    input.beginFrame();
    input.handleKeyPressed(GLFW_KEY_SPACE);
    input.handleKeyReleased(GLFW_KEY_SPACE);
    
    // State should be JustReleased (release overwrites press)
    REQUIRE(input.getKeyState(GLFW_KEY_SPACE) == InputState::JustReleased);
}