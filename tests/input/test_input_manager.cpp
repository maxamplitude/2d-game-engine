#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "input/input_manager.h"

using namespace Engine;

TEST_CASE("InputManager initial state", "[input]") {
    InputManager input;
    
    SECTION("All keys start as Up") {
        REQUIRE(input.getKeyState(sf::Keyboard::A) == InputState::Up);
        REQUIRE_FALSE(input.isKeyDown(sf::Keyboard::Space));
        REQUIRE_FALSE(input.isKeyPressed(sf::Keyboard::Space));
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
    input.handleKeyPressed(sf::Keyboard::Space);
    
    SECTION("Key is just pressed on first frame") {
        REQUIRE(input.getKeyState(sf::Keyboard::Space) == InputState::JustPressed);
        REQUIRE(input.isKeyPressed(sf::Keyboard::Space));
        REQUIRE(input.isKeyDown(sf::Keyboard::Space));
        REQUIRE_FALSE(input.isKeyReleased(sf::Keyboard::Space));
    }
    
    SECTION("Key becomes held on second frame") {
        input.update(0.016f);
        input.beginFrame();
        
        REQUIRE(input.getKeyState(sf::Keyboard::Space) == InputState::Held);
        REQUIRE_FALSE(input.isKeyPressed(sf::Keyboard::Space));
        REQUIRE(input.isKeyDown(sf::Keyboard::Space));
    }
}

TEST_CASE("InputManager tracks key release", "[input]") {
    InputManager input;
    
    // Press key
    input.beginFrame();
    input.handleKeyPressed(sf::Keyboard::Space);
    input.update(0.016f);
    
    // Hold for one frame
    input.beginFrame();
    input.update(0.016f);
    
    // Release key
    input.beginFrame();
    input.handleKeyReleased(sf::Keyboard::Space);
    
    SECTION("Key is just released") {
        REQUIRE(input.getKeyState(sf::Keyboard::Space) == InputState::JustReleased);
        REQUIRE(input.isKeyReleased(sf::Keyboard::Space));
        REQUIRE_FALSE(input.isKeyDown(sf::Keyboard::Space));
        REQUIRE_FALSE(input.isKeyPressed(sf::Keyboard::Space));
    }
    
    SECTION("Key becomes Up on next frame") {
        input.update(0.016f);
        input.beginFrame();
        
        REQUIRE(input.getKeyState(sf::Keyboard::Space) == InputState::Up);
        REQUIRE_FALSE(input.isKeyDown(sf::Keyboard::Space));
    }
}

TEST_CASE("InputManager action mapping", "[input]") {
    InputManager input;
    
    SECTION("Maps action to key") {
        input.mapAction("test_action", sf::Keyboard::Q);
        REQUIRE(input.hasKeyBinding("test_action"));
        REQUIRE(input.getKeyBinding("test_action") == sf::Keyboard::Q);
    }
    
    SECTION("Maps action to mouse button") {
        input.mapAction("test_action", sf::Mouse::Right);
        REQUIRE(input.hasMouseBinding("test_action"));
        REQUIRE(input.getMouseBinding("test_action") == sf::Mouse::Right);
    }
    
    SECTION("Remapping replaces existing binding") {
        input.mapAction("jump", sf::Keyboard::Space);
        input.mapAction("jump", sf::Keyboard::W);
        
        REQUIRE(input.getKeyBinding("jump") == sf::Keyboard::W);
    }
    
    SECTION("Can unmap action") {
        input.mapAction("test", sf::Keyboard::T);
        input.unmapAction("test");
        
        REQUIRE_FALSE(input.hasKeyBinding("test"));
    }
}

TEST_CASE("InputManager action queries", "[input]") {
    InputManager input;
    input.mapAction("test_action", sf::Keyboard::T);
    
    SECTION("Action is active when key is down") {
        input.beginFrame();
        input.handleKeyPressed(sf::Keyboard::T);
        
        REQUIRE(input.isActionActive("test_action"));
        REQUIRE(input.isActionPressed("test_action"));
    }
    
    SECTION("Action pressed only on first frame") {
        input.beginFrame();
        input.handleKeyPressed(sf::Keyboard::T);
        REQUIRE(input.isActionPressed("test_action"));
        
        input.update(0.016f);
        input.beginFrame();
        REQUIRE_FALSE(input.isActionPressed("test_action"));
        REQUIRE(input.isActionActive("test_action"));  // Still held
    }
    
    SECTION("Action released detection") {
        // Press
        input.beginFrame();
        input.handleKeyPressed(sf::Keyboard::T);
        input.update(0.016f);
        
        // Release
        input.beginFrame();
        input.handleKeyReleased(sf::Keyboard::T);
        
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
        input.handleMouseButtonPressed(sf::Mouse::Left);
        
        REQUIRE(input.isMouseButtonPressed(sf::Mouse::Left));
        REQUIRE(input.isMouseButtonDown(sf::Mouse::Left));
        REQUIRE(input.getMouseButtonState(sf::Mouse::Left) == InputState::JustPressed);
    }
    
    SECTION("Mouse button becomes held") {
        input.beginFrame();
        input.handleMouseButtonPressed(sf::Mouse::Left);
        input.update(0.016f);
        
        input.beginFrame();
        
        REQUIRE(input.getMouseButtonState(sf::Mouse::Left) == InputState::Held);
        REQUIRE_FALSE(input.isMouseButtonPressed(sf::Mouse::Left));
        REQUIRE(input.isMouseButtonDown(sf::Mouse::Left));
    }
    
    SECTION("Tracks mouse button release") {
        // Press
        input.beginFrame();
        input.handleMouseButtonPressed(sf::Mouse::Left);
        input.update(0.016f);
        
        // Release
        input.beginFrame();
        input.handleMouseButtonReleased(sf::Mouse::Left);
        
        REQUIRE(input.isMouseButtonReleased(sf::Mouse::Left));
        REQUIRE_FALSE(input.isMouseButtonDown(sf::Mouse::Left));
    }
}

TEST_CASE("InputManager action callbacks", "[input]") {
    InputManager input;
    input.mapAction("test_action", sf::Keyboard::T);
    
    bool callbackFired = false;
    input.setActionPressedCallback("test_action", [&]() {
        callbackFired = true;
    });
    
    SECTION("Callback fires when action pressed") {
        input.beginFrame();
        input.handleKeyPressed(sf::Keyboard::T);
        input.update(0.016f);  // Triggers callbacks
        
        REQUIRE(callbackFired);
    }
    
    SECTION("Callback doesn't fire when action held") {
        // Press
        input.beginFrame();
        input.handleKeyPressed(sf::Keyboard::T);
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
        input.handleKeyPressed(sf::Keyboard::T);
        input.update(0.016f);
        
        REQUIRE_FALSE(callbackFired);
    }
}

TEST_CASE("InputManager multiple keys for same action", "[input]") {
    InputManager input;
    
    // Note: Current design only supports one binding per action
    // If you need multiple bindings, you'd need to track multiple keys per action
    
    input.mapAction("move_right", sf::Keyboard::D);
    input.mapAction("move_right_alt", sf::Keyboard::Right);
    
    input.beginFrame();
    input.handleKeyPressed(sf::Keyboard::D);
    
    REQUIRE(input.isActionActive("move_right"));
    REQUIRE_FALSE(input.isActionActive("move_right_alt"));
}

TEST_CASE("InputManager prevents double buffering active actions", "[input]") {
    InputManager input;
    input.mapAction("jump", sf::Keyboard::Space);
    
    input.beginFrame();
    input.handleKeyPressed(sf::Keyboard::Space);
    
    // Try to buffer while action is active
    input.bufferAction("jump", 0.1f);
    
    // Release and try to consume buffer
    input.update(0.016f);
    input.beginFrame();
    input.handleKeyReleased(sf::Keyboard::Space);
    input.update(0.016f);
    
    // Buffer shouldn't exist because action was active when buffered
    REQUIRE_FALSE(input.consumeBufferedAction("jump"));
}

TEST_CASE("InputManager clear all mappings", "[input]") {
    InputManager input;
    
    input.mapAction("action1", sf::Keyboard::A);
    input.mapAction("action2", sf::Keyboard::B);
    input.mapAction("action3", sf::Mouse::Left);
    
    input.clearAllMappings();
    
    REQUIRE_FALSE(input.hasKeyBinding("action1"));
    REQUIRE_FALSE(input.hasKeyBinding("action2"));
    REQUIRE_FALSE(input.hasMouseBinding("action3"));
}

TEST_CASE("InputManager handles rapid press-release", "[input]") {
    InputManager input;
    
    // Press and release in same frame (before update)
    input.beginFrame();
    input.handleKeyPressed(sf::Keyboard::Space);
    input.handleKeyReleased(sf::Keyboard::Space);
    
    // State should be JustReleased (release overwrites press)
    REQUIRE(input.getKeyState(sf::Keyboard::Space) == InputState::JustReleased);
}