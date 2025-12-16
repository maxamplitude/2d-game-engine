#include "input_manager.h"
#include <algorithm>

namespace Engine {

InputManager::InputManager() {
    // Initialize with some common default mappings
    mapAction("jump", sf::Keyboard::Space);
    mapAction("move_left", sf::Keyboard::A);
    mapAction("move_right", sf::Keyboard::D);
    mapAction("move_up", sf::Keyboard::W);
    mapAction("move_down", sf::Keyboard::S);
    mapAction("interact", sf::Keyboard::E);
    mapAction("attack", sf::Mouse::Left);
}

void InputManager::beginFrame() {
    // Save previous frame's states
    previousKeyStates = keyStates;
    previousMouseStates = mouseStates;
    
    // Reset just pressed/released states to held/up
    for (auto& [key, state] : keyStates) {
        if (state == InputState::JustPressed) {
            state = InputState::Held;
        } else if (state == InputState::JustReleased) {
            state = InputState::Up;
        }
    }
    
    for (auto& [button, state] : mouseStates) {
        if (state == InputState::JustPressed) {
            state = InputState::Held;
        } else if (state == InputState::JustReleased) {
            state = InputState::Up;
        }
    }
}

void InputManager::update(float dt) {
    updateBufferedInputs(dt);
    triggerActionCallbacks();
}

void InputManager::handleKeyPressed(sf::Keyboard::Key key) {
    auto& state = keyStates[key];
    if (state == InputState::Up || state == InputState::JustReleased) {
        state = InputState::JustPressed;
    }
}

void InputManager::handleKeyReleased(sf::Keyboard::Key key) {
    auto& state = keyStates[key];
    if (state == InputState::Held || state == InputState::JustPressed) {
        state = InputState::JustReleased;
    }
}

void InputManager::handleMouseButtonPressed(sf::Mouse::Button button) {
    auto& state = mouseStates[button];
    if (state == InputState::Up || state == InputState::JustReleased) {
        state = InputState::JustPressed;
    }
}

void InputManager::handleMouseButtonReleased(sf::Mouse::Button button) {
    auto& state = mouseStates[button];
    if (state == InputState::Held || state == InputState::JustPressed) {
        state = InputState::JustReleased;
    }
}

bool InputManager::isKeyDown(sf::Keyboard::Key key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == InputState::JustPressed || it->second == InputState::Held;
}

bool InputManager::isKeyPressed(sf::Keyboard::Key key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == InputState::JustPressed;
}

bool InputManager::isKeyReleased(sf::Keyboard::Key key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == InputState::JustReleased;
}

InputState InputManager::getKeyState(sf::Keyboard::Key key) const {
    auto it = keyStates.find(key);
    return (it != keyStates.end()) ? it->second : InputState::Up;
}

bool InputManager::isMouseButtonDown(sf::Mouse::Button button) const {
    auto it = mouseStates.find(button);
    if (it == mouseStates.end()) return false;
    return it->second == InputState::JustPressed || it->second == InputState::Held;
}

bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const {
    auto it = mouseStates.find(button);
    if (it == mouseStates.end()) return false;
    return it->second == InputState::JustPressed;
}

bool InputManager::isMouseButtonReleased(sf::Mouse::Button button) const {
    auto it = mouseStates.find(button);
    if (it == mouseStates.end()) return false;
    return it->second == InputState::JustReleased;
}

InputState InputManager::getMouseButtonState(sf::Mouse::Button button) const {
    auto it = mouseStates.find(button);
    return (it != mouseStates.end()) ? it->second : InputState::Up;
}

void InputManager::mapAction(const std::string& action, sf::Keyboard::Key key) {
    // Remove from mouse mapping if it exists
    actionToMouse.erase(action);
    actionToKey[action] = key;
}

void InputManager::mapAction(const std::string& action, sf::Mouse::Button button) {
    // Remove from key mapping if it exists
    actionToKey.erase(action);
    actionToMouse[action] = button;
}

void InputManager::unmapAction(const std::string& action) {
    actionToKey.erase(action);
    actionToMouse.erase(action);
}

void InputManager::clearAllMappings() {
    actionToKey.clear();
    actionToMouse.clear();
}

bool InputManager::isActionActive(const std::string& action) const {
    // Check key binding
    auto keyIt = actionToKey.find(action);
    if (keyIt != actionToKey.end() && isKeyDown(keyIt->second)) {
        return true;
    }
    
    // Check mouse binding
    auto mouseIt = actionToMouse.find(action);
    if (mouseIt != actionToMouse.end() && isMouseButtonDown(mouseIt->second)) {
        return true;
    }
    
    return false;
}

bool InputManager::isActionPressed(const std::string& action) const {
    // Check key binding
    auto keyIt = actionToKey.find(action);
    if (keyIt != actionToKey.end() && isKeyPressed(keyIt->second)) {
        return true;
    }
    
    // Check mouse binding
    auto mouseIt = actionToMouse.find(action);
    if (mouseIt != actionToMouse.end() && isMouseButtonPressed(mouseIt->second)) {
        return true;
    }
    
    return false;
}

bool InputManager::isActionReleased(const std::string& action) const {
    // Check key binding
    auto keyIt = actionToKey.find(action);
    if (keyIt != actionToKey.end() && isKeyReleased(keyIt->second)) {
        return true;
    }
    
    // Check mouse binding
    auto mouseIt = actionToMouse.find(action);
    if (mouseIt != actionToMouse.end() && isMouseButtonReleased(mouseIt->second)) {
        return true;
    }
    
    return false;
}

void InputManager::bufferAction(const std::string& action, float bufferTime) {
    // Don't buffer if action is currently active (avoid double-buffering)
    if (isActionActive(action)) {
        return;
    }
    
    // Check if action is already buffered, update its time
    for (auto& buffered : inputBuffer) {
        if (buffered.action == action) {
            buffered.timeRemaining = bufferTime;
            return;
        }
    }
    
    // Add new buffered input
    inputBuffer.emplace_back(action, bufferTime);
}

bool InputManager::consumeBufferedAction(const std::string& action) {
    for (auto it = inputBuffer.begin(); it != inputBuffer.end(); ++it) {
        if (it->action == action) {
            inputBuffer.erase(it);
            return true;
        }
    }
    return false;
}

void InputManager::clearBuffer() {
    inputBuffer.clear();
}

sf::Keyboard::Key InputManager::getKeyBinding(const std::string& action) const {
    auto it = actionToKey.find(action);
    return (it != actionToKey.end()) ? it->second : sf::Keyboard::Unknown;
}

sf::Mouse::Button InputManager::getMouseBinding(const std::string& action) const {
    auto it = actionToMouse.find(action);
    return (it != actionToMouse.end()) ? it->second : sf::Mouse::ButtonCount;
}

bool InputManager::hasKeyBinding(const std::string& action) const {
    return actionToKey.find(action) != actionToKey.end();
}

bool InputManager::hasMouseBinding(const std::string& action) const {
    return actionToMouse.find(action) != actionToMouse.end();
}

void InputManager::setActionPressedCallback(const std::string& action, 
                                            std::function<void()> callback) {
    actionCallbacks[action] = callback;
}

void InputManager::clearActionCallbacks() {
    actionCallbacks.clear();
}

void InputManager::updateKeyState(sf::Keyboard::Key key) {
    bool isDown = sf::Keyboard::isKeyPressed(key);
    auto& state = keyStates[key];
    
    if (isDown) {
        if (state == InputState::Up || state == InputState::JustReleased) {
            state = InputState::JustPressed;
        } else {
            state = InputState::Held;
        }
    } else {
        if (state == InputState::Held || state == InputState::JustPressed) {
            state = InputState::JustReleased;
        } else {
            state = InputState::Up;
        }
    }
}

void InputManager::updateMouseState(sf::Mouse::Button button) {
    bool isDown = sf::Mouse::isButtonPressed(button);
    auto& state = mouseStates[button];
    
    if (isDown) {
        if (state == InputState::Up || state == InputState::JustReleased) {
            state = InputState::JustPressed;
        } else {
            state = InputState::Held;
        }
    } else {
        if (state == InputState::Held || state == InputState::JustPressed) {
            state = InputState::JustReleased;
        } else {
            state = InputState::Up;
        }
    }
}

void InputManager::updateBufferedInputs(float dt) {
    // Decay all buffered inputs
    for (auto& buffered : inputBuffer) {
        buffered.timeRemaining -= dt;
    }
    
    // Remove expired buffers
    inputBuffer.erase(
        std::remove_if(inputBuffer.begin(), inputBuffer.end(),
            [](const BufferedInput& b) { return b.timeRemaining <= 0.0f; }),
        inputBuffer.end()
    );
}

void InputManager::triggerActionCallbacks() {
    for (const auto& [action, callback] : actionCallbacks) {
        if (isActionPressed(action)) {
            callback();
        }
    }
}

} // namespace Engine