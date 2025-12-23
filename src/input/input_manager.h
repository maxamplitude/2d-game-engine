#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <string>
#include <queue>
#include <functional>

namespace Engine {

// Input state for a single key/button
enum class InputState {
    Up,            // Not pressed
    JustPressed,   // Pressed this frame
    Held,          // Held down (pressed for multiple frames)
    JustReleased   // Released this frame
};

// Buffered input for platformer-style input windows
struct BufferedInput {
    std::string action;
    float timeRemaining;  // How long this buffer is valid
    
    BufferedInput(const std::string& a, float time) 
        : action(a), timeRemaining(time) {}
};

class InputManager {
public:
    InputManager();
    explicit InputManager(GLFWwindow* window);

    void attachWindow(GLFWwindow* window);

    void beginFrame();
    void update(float dt);

    void handleKeyPressed(int key);
    void handleKeyReleased(int key);
    void handleMouseButtonPressed(int button);
    void handleMouseButtonReleased(int button);

    bool isKeyDown(int key) const;
    bool isKeyPressed(int key) const;
    bool isKeyReleased(int key) const;
    InputState getKeyState(int key) const;

    bool isMouseButtonDown(int button) const;
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    InputState getMouseButtonState(int button) const;

    void mapAction(const std::string& action, int key);
    void mapActionMouse(const std::string& action, int button);
    void unmapAction(const std::string& action);
    void clearAllMappings();

    bool isActionActive(const std::string& action) const;
    bool isActionPressed(const std::string& action) const;
    bool isActionReleased(const std::string& action) const;

    void bufferAction(const std::string& action, float bufferTime = 0.1f);
    bool consumeBufferedAction(const std::string& action);
    void clearBuffer();

    int getKeyBinding(const std::string& action) const;
    int getMouseBinding(const std::string& action) const;
    bool hasKeyBinding(const std::string& action) const;
    bool hasMouseBinding(const std::string& action) const;

    void setActionPressedCallback(const std::string& action,
                                  std::function<void()> callback);
    void clearActionCallbacks();

private:
    std::unordered_map<int, InputState> keyStates;
    std::unordered_map<int, InputState> previousKeyStates;

    std::unordered_map<int, InputState> mouseStates;
    std::unordered_map<int, InputState> previousMouseStates;

    std::unordered_map<std::string, int> actionToKey;
    std::unordered_map<std::string, int> actionToMouse;

    std::vector<BufferedInput> inputBuffer;
    std::unordered_map<std::string, std::function<void()>> actionCallbacks;

    GLFWwindow* windowHandle = nullptr;

    void updateKeyState(int key);
    void updateMouseState(int button);
    void updateBufferedInputs(float dt);
    void triggerActionCallbacks();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static std::unordered_map<GLFWwindow*, InputManager*> s_instances;
};

} // namespace Engine