#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
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
    
    // Must be called each frame BEFORE event polling
    void beginFrame();
    
    // Must be called each frame AFTER event polling
    void update(float dt);
    
    // Event handling (call from event loop)
    void handleKeyPressed(sf::Keyboard::Key key);
    void handleKeyReleased(sf::Keyboard::Key key);
    void handleMouseButtonPressed(sf::Mouse::Button button);
    void handleMouseButtonReleased(sf::Mouse::Button button);
    
    // Raw key state queries
    bool isKeyDown(sf::Keyboard::Key key) const;
    bool isKeyPressed(sf::Keyboard::Key key) const;   // Just pressed this frame
    bool isKeyReleased(sf::Keyboard::Key key) const;  // Just released this frame
    InputState getKeyState(sf::Keyboard::Key key) const;
    
    // Raw mouse button queries
    bool isMouseButtonDown(sf::Mouse::Button button) const;
    bool isMouseButtonPressed(sf::Mouse::Button button) const;
    bool isMouseButtonReleased(sf::Mouse::Button button) const;
    InputState getMouseButtonState(sf::Mouse::Button button) const;
    
    // Action mapping (bind actions to keys)
    void mapAction(const std::string& action, sf::Keyboard::Key key);
    void mapAction(const std::string& action, sf::Mouse::Button button);
    void unmapAction(const std::string& action);
    void clearAllMappings();
    
    // Action queries (uses mapped keys)
    bool isActionActive(const std::string& action) const;
    bool isActionPressed(const std::string& action) const;
    bool isActionReleased(const std::string& action) const;
    
    // Input buffering (for jump buffering, etc.)
    void bufferAction(const std::string& action, float bufferTime = 0.1f);
    bool consumeBufferedAction(const std::string& action);
    void clearBuffer();
    
    // Get current key binding for an action (for UI display)
    sf::Keyboard::Key getKeyBinding(const std::string& action) const;
    sf::Mouse::Button getMouseBinding(const std::string& action) const;
    bool hasKeyBinding(const std::string& action) const;
    bool hasMouseBinding(const std::string& action) const;
    
    // Callbacks for when specific actions occur
    void setActionPressedCallback(const std::string& action, 
                                  std::function<void()> callback);
    void clearActionCallbacks();
    
private:
    // Key state tracking
    std::unordered_map<sf::Keyboard::Key, InputState> keyStates;
    std::unordered_map<sf::Keyboard::Key, InputState> previousKeyStates;
    
    // Mouse button state tracking
    std::unordered_map<sf::Mouse::Button, InputState> mouseStates;
    std::unordered_map<sf::Mouse::Button, InputState> previousMouseStates;
    
    // Action mapping
    std::unordered_map<std::string, sf::Keyboard::Key> actionToKey;
    std::unordered_map<std::string, sf::Mouse::Button> actionToMouse;
    
    // Input buffering
    std::vector<BufferedInput> inputBuffer;
    
    // Action callbacks
    std::unordered_map<std::string, std::function<void()>> actionCallbacks;
    
    // Helper methods
    void updateKeyState(sf::Keyboard::Key key);
    void updateMouseState(sf::Mouse::Button button);
    void updateBufferedInputs(float dt);
    void triggerActionCallbacks();
};

} // namespace Engine