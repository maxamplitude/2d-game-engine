#include "input_manager.h"
#include <algorithm>

namespace Engine {

std::unordered_map<GLFWwindow*, InputManager*> InputManager::s_instances;

InputManager::InputManager() {
    mapAction("jump", GLFW_KEY_SPACE);
    mapAction("move_left", GLFW_KEY_A);
    mapAction("move_right", GLFW_KEY_D);
    mapAction("move_up", GLFW_KEY_W);
    mapAction("move_down", GLFW_KEY_S);
    mapActionMouse("attack", GLFW_MOUSE_BUTTON_LEFT);
}

InputManager::InputManager(GLFWwindow* window) : InputManager() {
    attachWindow(window);
}

void InputManager::attachWindow(GLFWwindow* window) {
    windowHandle = window;
    if (!window) return;
    s_instances[window] = this;
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

void InputManager::beginFrame() {
    previousKeyStates = keyStates;
    previousMouseStates = mouseStates;

    for (auto& [key, state] : keyStates) {
        if (state == InputState::JustPressed) state = InputState::Held;
        else if (state == InputState::JustReleased) state = InputState::Up;
    }

    for (auto& [button, state] : mouseStates) {
        if (state == InputState::JustPressed) state = InputState::Held;
        else if (state == InputState::JustReleased) state = InputState::Up;
    }
}

void InputManager::update(float dt) {
    updateBufferedInputs(dt);
    triggerActionCallbacks();
}

void InputManager::handleKeyPressed(int key) {
    auto& state = keyStates[key];
    if (state == InputState::Up || state == InputState::JustReleased) {
        state = InputState::JustPressed;
    }
}

void InputManager::handleKeyReleased(int key) {
    auto& state = keyStates[key];
    if (state == InputState::Held || state == InputState::JustPressed) {
        state = InputState::JustReleased;
    }
}

void InputManager::handleMouseButtonPressed(int button) {
    auto& state = mouseStates[button];
    if (state == InputState::Up || state == InputState::JustReleased) {
        state = InputState::JustPressed;
    }
}

void InputManager::handleMouseButtonReleased(int button) {
    auto& state = mouseStates[button];
    if (state == InputState::Held || state == InputState::JustPressed) {
        state = InputState::JustReleased;
    }
}

bool InputManager::isKeyDown(int key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == InputState::JustPressed || it->second == InputState::Held;
}

bool InputManager::isKeyPressed(int key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == InputState::JustPressed;
}

bool InputManager::isKeyReleased(int key) const {
    auto it = keyStates.find(key);
    if (it == keyStates.end()) return false;
    return it->second == InputState::JustReleased;
}

InputState InputManager::getKeyState(int key) const {
    auto it = keyStates.find(key);
    return (it != keyStates.end()) ? it->second : InputState::Up;
}

bool InputManager::isMouseButtonDown(int button) const {
    auto it = mouseStates.find(button);
    if (it == mouseStates.end()) return false;
    return it->second == InputState::JustPressed || it->second == InputState::Held;
}

bool InputManager::isMouseButtonPressed(int button) const {
    auto it = mouseStates.find(button);
    if (it == mouseStates.end()) return false;
    return it->second == InputState::JustPressed;
}

bool InputManager::isMouseButtonReleased(int button) const {
    auto it = mouseStates.find(button);
    if (it == mouseStates.end()) return false;
    return it->second == InputState::JustReleased;
}

InputState InputManager::getMouseButtonState(int button) const {
    auto it = mouseStates.find(button);
    return (it != mouseStates.end()) ? it->second : InputState::Up;
}

void InputManager::mapAction(const std::string& action, int key) {
    actionToMouse.erase(action);
    actionToKey[action] = key;
}

void InputManager::mapActionMouse(const std::string& action, int button) {
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
    auto keyIt = actionToKey.find(action);
    if (keyIt != actionToKey.end() && isKeyDown(keyIt->second)) return true;

    auto mouseIt = actionToMouse.find(action);
    if (mouseIt != actionToMouse.end() && isMouseButtonDown(mouseIt->second)) return true;

    return false;
}

bool InputManager::isActionPressed(const std::string& action) const {
    auto keyIt = actionToKey.find(action);
    if (keyIt != actionToKey.end() && isKeyPressed(keyIt->second)) return true;

    auto mouseIt = actionToMouse.find(action);
    if (mouseIt != actionToMouse.end() && isMouseButtonPressed(mouseIt->second)) return true;

    return false;
}

bool InputManager::isActionReleased(const std::string& action) const {
    auto keyIt = actionToKey.find(action);
    if (keyIt != actionToKey.end() && isKeyReleased(keyIt->second)) return true;

    auto mouseIt = actionToMouse.find(action);
    if (mouseIt != actionToMouse.end() && isMouseButtonReleased(mouseIt->second)) return true;

    return false;
}

void InputManager::bufferAction(const std::string& action, float bufferTime) {
    if (isActionActive(action)) return;

    for (auto& input : inputBuffer) {
        if (input.action == action) {
            input.timeRemaining = bufferTime;
            return;
        }
    }
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

int InputManager::getKeyBinding(const std::string& action) const {
    auto it = actionToKey.find(action);
    return (it != actionToKey.end()) ? it->second : GLFW_KEY_UNKNOWN;
}

int InputManager::getMouseBinding(const std::string& action) const {
    auto it = actionToMouse.find(action);
    return (it != actionToMouse.end()) ? it->second : -1;
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

void InputManager::updateKeyState(int key) {
    if (!windowHandle) return;
    int state = glfwGetKey(windowHandle, key);
    bool isDown = (state == GLFW_PRESS || state == GLFW_REPEAT);
    auto& s = keyStates[key];

    if (isDown) {
        s = (s == InputState::Up || s == InputState::JustReleased) ? InputState::JustPressed : InputState::Held;
    } else {
        s = (s == InputState::Held || s == InputState::JustPressed) ? InputState::JustReleased : InputState::Up;
    }
}

void InputManager::updateMouseState(int button) {
    if (!windowHandle) return;
    int state = glfwGetMouseButton(windowHandle, button);
    bool isDown = (state == GLFW_PRESS);
    auto& s = mouseStates[button];

    if (isDown) {
        s = (s == InputState::Up || s == InputState::JustReleased) ? InputState::JustPressed : InputState::Held;
    } else {
        s = (s == InputState::Held || s == InputState::JustPressed) ? InputState::JustReleased : InputState::Up;
    }
}

void InputManager::updateBufferedInputs(float dt) {
    for (auto it = inputBuffer.begin(); it != inputBuffer.end(); ) {
        it->timeRemaining -= dt;
        if (it->timeRemaining <= 0.0f) {
            it = inputBuffer.erase(it);
        } else {
            ++it;
        }
    }
}

void InputManager::triggerActionCallbacks() {
    for (const auto& [action, callback] : actionCallbacks) {
        if (callback && isActionPressed(action)) {
            callback();
        }
    }
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto it = s_instances.find(window);
    if (it == s_instances.end()) return;
    InputManager* self = it->second;
    if (action == GLFW_PRESS) self->handleKeyPressed(key);
    else if (action == GLFW_RELEASE) self->handleKeyReleased(key);
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto it = s_instances.find(window);
    if (it == s_instances.end()) return;
    InputManager* self = it->second;
    if (action == GLFW_PRESS) self->handleMouseButtonPressed(button);
    else if (action == GLFW_RELEASE) self->handleMouseButtonReleased(button);
}

} // namespace Engine