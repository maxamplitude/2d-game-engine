// examples/scene_example.cpp
// Complete usage example for the scene management system

#include "scene/scene_manager.h"
#include "scene/scene.h"
#include "core/time_manager.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace Engine;

// ============================================================================
// Example Game Scenes
// ============================================================================

/**
 * Main Menu Scene
 * Shows how to implement a simple menu with scene transitions
 */
class MenuScene : public Scene {
private:
    sf::Font font;
    sf::Text titleText;
    sf::Text instructionsText;
    
public:
    void onEnter() override {
        std::cout << "MenuScene: Entered" << std::endl;
        
        // Setup UI (font loading would normally be done via resource manager)
        titleText.setString("MAIN MENU");
        titleText.setCharacterSize(48);
        titleText.setPosition(300, 200);
        
        instructionsText.setString("Press SPACE to start game");
        instructionsText.setCharacterSize(24);
        instructionsText.setPosition(250, 300);
    }
    
    void onExit() override {
        std::cout << "MenuScene: Exited" << std::endl;
    }
    
    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Space) {
                // Transition to gameplay with fade
                TransitionConfig fade(SceneTransition::Fade, 0.5f);
                auto gameplay = manager->createScene<GameplayScene>();
                manager->changeScene(std::move(gameplay), fade);
            }
        }
    }
    
    void update(float dt) override {
        // Menu logic (could animate text, etc.)
    }
    
    void render(sf::RenderWindow& window) override {
        window.draw(titleText);
        window.draw(instructionsText);
    }
};

/**
 * Gameplay Scene
 * Shows active gameplay with pause menu support
 */
class GameplayScene : public Scene {
private:
    sf::CircleShape player;
    sf::Vector2f playerVelocity;
    const float playerSpeed = 200.0f;
    
public:
    void onEnter() override {
        std::cout << "GameplayScene: Entered" << std::endl;
        
        // Initialize game state
        player.setRadius(30.0f);
        player.setFillColor(sf::Color::Green);
        player.setPosition(400, 300);
        player.setOrigin(30, 30);
    }
    
    void onPause() override {
        std::cout << "GameplayScene: Paused (menu opened)" << std::endl;
    }
    
    void onResume() override {
        std::cout << "GameplayScene: Resumed" << std::endl;
    }
    
    void onExit() override {
        std::cout << "GameplayScene: Exited" << std::endl;
    }
    
    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                // Open pause menu (pushes on stack)
                auto pauseMenu = manager->createScene<PauseScene>();
                manager->pushScene(std::move(pauseMenu));
            }
        }
    }
    
    void update(float dt) override {
        // Player movement
        playerVelocity = {0, 0};
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            playerVelocity.x -= playerSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            playerVelocity.x += playerSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            playerVelocity.y -= playerSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            playerVelocity.y += playerSpeed;
        
        player.move(playerVelocity * dt);
    }
    
    void render(sf::RenderWindow& window) override {
        window.draw(player);
    }
};

/**
 * Pause Scene
 * Shows overlay scene that pauses gameplay
 */
class PauseScene : public Scene {
private:
    sf::RectangleShape overlay;
    sf::Text pausedText;
    
public:
    void onEnter() override {
        std::cout << "PauseScene: Entered" << std::endl;
        
        // Semi-transparent overlay
        overlay.setSize(sf::Vector2f(800, 600));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        
        pausedText.setString("PAUSED\n\nPress ESC to resume\nPress Q to quit");
        pausedText.setCharacterSize(32);
        pausedText.setPosition(300, 250);
    }
    
    void onExit() override {
        std::cout << "PauseScene: Exited" << std::endl;
    }
    
    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                // Resume gameplay (pop this scene)
                manager->popScene();
            }
            else if (event.key.code == sf::Keyboard::Q) {
                // Return to main menu (change scene removes entire stack)
                TransitionConfig fade(SceneTransition::Fade, 0.3f);
                auto menu = manager->createScene<MenuScene>();
                manager->changeScene(std::move(menu), fade);
            }
        }
    }
    
    void update(float dt) override {
        // Pause menu logic (could animate, etc.)
    }
    
    void render(sf::RenderWindow& window) override {
        window.draw(overlay);
        window.draw(pausedText);
    }
};

// ============================================================================
// Main Game Loop Integration
// ============================================================================

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Scene Management Example");
    window.setFramerateLimit(60);
    
    // Initialize systems
    TimeManager timeManager;
    SceneManager sceneManager;
    
    // Start with main menu
    auto mainMenu = sceneManager.createScene<MenuScene>();
    sceneManager.changeScene(std::move(mainMenu));
    
    // Game loop
    while (window.isOpen()) {
        timeManager.update();
        float dt = timeManager.getDeltaTime();
        
        // Event handling
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            // Let scene manager handle events
            sceneManager.handleEvent(event);
        }
        
        // Update current scene
        sceneManager.update(dt);
        
        // Render
        window.clear(sf::Color(20, 20, 30));
        sceneManager.render(window);
        window.display();
    }
    
    return 0;
}

// ============================================================================
// Advanced Usage Examples
// ============================================================================

/**
 * Example: Loading Screen Scene
 * Shows how to create a scene that loads resources asynchronously
 */
class LoadingScene : public Scene {
private:
    sf::Text loadingText;
    std::vector<std::string> assetsToLoad;
    size_t currentAssetIndex = 0;
    
public:
    void onEnter() override {
        loadingText.setString("Loading...");
        assetsToLoad = {"texture1", "texture2", "sound1", "music1"};
        currentAssetIndex = 0;
    }
    
    void update(float dt) override {
        // Simulate loading one asset per frame
        if (currentAssetIndex < assetsToLoad.size()) {
            std::cout << "Loading: " << assetsToLoad[currentAssetIndex] << std::endl;
            currentAssetIndex++;
        } else {
            // Loading complete - transition to game
            TransitionConfig fade(SceneTransition::Fade, 0.5f);
            auto gameplay = manager->createScene<GameplayScene>();
            manager->changeScene(std::move(gameplay), fade);
        }
    }
    
    void render(sf::RenderWindow& window) override {
        window.draw(loadingText);
        
        // Draw progress bar
        float progress = static_cast<float>(currentAssetIndex) / assetsToLoad.size();
        sf::RectangleShape progressBar(sf::Vector2f(400 * progress, 30));
        progressBar.setPosition(200, 300);
        progressBar.setFillColor(sf::Color::Green);
        window.draw(progressBar);
    }
};

/**
 * Example: Dialogue Scene
 * Shows how to create a temporary overlay scene
 */
class DialogueScene : public Scene {
private:
    sf::RectangleShape dialogueBox;
    sf::Text dialogueText;
    std::vector<std::string> lines;
    size_t currentLine = 0;
    
public:
    DialogueScene(const std::vector<std::string>& dialogueLines) 
        : lines(dialogueLines) {}
    
    void onEnter() override {
        dialogueBox.setSize(sf::Vector2f(700, 150));
        dialogueBox.setPosition(50, 400);
        dialogueBox.setFillColor(sf::Color(0, 0, 0, 200));
        
        updateDialogue();
    }
    
    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Space) {
                currentLine++;
                if (currentLine >= lines.size()) {
                    // Dialogue finished - remove this scene
                    manager->popScene();
                } else {
                    updateDialogue();
                }
            }
        }
    }
    
    void update(float dt) override {}
    
    void render(sf::RenderWindow& window) override {
        window.draw(dialogueBox);
        window.draw(dialogueText);
    }
    
private:
    void updateDialogue() {
        if (currentLine < lines.size()) {
            dialogueText.setString(lines[currentLine]);
            dialogueText.setCharacterSize(20);
            dialogueText.setPosition(70, 420);
        }
    }
};