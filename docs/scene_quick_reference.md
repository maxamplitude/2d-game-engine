# Scene Management - Quick Reference

## Class Hierarchy

```
Scene (abstract)
├── onEnter()       - Scene becomes active
├── onExit()        - Scene is removed
├── onPause()       - Another scene pushed on top
├── onResume()      - Top scene popped, this reactivates
├── handleEvent()   - Process SFML events
├── update(dt)      - Frame update (pure virtual)
└── render(window)  - Draw to window (pure virtual)

SceneManager
├── changeScene()   - Replace current scene(s)
├── pushScene()     - Add scene on top (pause current)
├── popScene()      - Remove top scene (resume previous)
├── update(dt)      - Update active scene
├── render(window)  - Render all scenes
└── createScene<T>() - Type-safe scene creation
```

## Common Operations

### Change Scene (Replace)
```cpp
TransitionConfig fade(SceneTransition::Fade, 0.5f);
auto newScene = manager.createScene<GameplayScene>();
manager.changeScene(std::move(newScene), fade);
```
**Result:** Old scene removed, new scene active
**Lifecycle:** oldScene->onExit(), newScene->onEnter()

### Push Scene (Overlay)
```cpp
auto pause = manager.createScene<PauseScene>();
manager.pushScene(std::move(pause));
```
**Result:** Old scene paused, new scene on top
**Lifecycle:** oldScene->onPause(), newScene->onEnter()
**Stack:** [OldScene, NewScene] ← NewScene is active

### Pop Scene (Remove Top)
```cpp
manager.popScene();
```
**Result:** Top scene removed, previous becomes active
**Lifecycle:** topScene->onExit(), previousScene->onResume()
**Stack:** [Scene1, Scene2] → popScene() → [Scene1] ← Scene1 is active

## Scene Stack Visualization

```
Operation: changeScene(Menu)
Stack: [Menu]
Active: Menu

Operation: changeScene(Gameplay)
Stack: [Gameplay]
Active: Gameplay

Operation: pushScene(Pause)
Stack: [Gameplay, Pause]
Active: Pause (Gameplay paused but still rendering)

Operation: pushScene(Settings)
Stack: [Gameplay, Pause, Settings]
Active: Settings

Operation: popScene()
Stack: [Gameplay, Pause]
Active: Pause (Settings gone, Pause resumed)

Operation: popScene()
Stack: [Gameplay]
Active: Gameplay (Pause gone, Gameplay resumed)

Operation: changeScene(Menu)
Stack: [Menu]
Active: Menu (Gameplay gone, cleared entire stack)
```

## Transition Types

```cpp
// Instant (no fade)
TransitionConfig instant(SceneTransition::None);

// Black fade (default)
TransitionConfig blackFade(SceneTransition::Fade, 0.5f);

// White fade
TransitionConfig whiteFade(
    SceneTransition::Fade, 
    0.3f, 
    sf::Color::White
);

// Red fade
TransitionConfig redFade(
    SceneTransition::Fade, 
    0.2f, 
    sf::Color(255, 0, 0)
);
```

## Typical Scene Patterns

### Main Menu
```cpp
class MenuScene : public Scene {
    void handleEvent(const sf::Event& event) override {
        if (spacePressed) {
            auto game = manager->createScene<GameplayScene>();
            manager->changeScene(std::move(game), fade);
        }
    }
};
```

### Gameplay
```cpp
class GameplayScene : public Scene {
    void handleEvent(const sf::Event& event) override {
        if (escapePressed) {
            auto pause = manager->createScene<PauseScene>();
            manager->pushScene(std::move(pause));
        }
    }
    
    void update(float dt) override {
        if (playerDied) {
            auto gameOver = manager->createScene<GameOverScene>(score);
            manager->changeScene(std::move(gameOver));
        }
    }
};
```

### Pause Menu
```cpp
class PauseScene : public Scene {
    void render(sf::RenderWindow& window) override {
        // Semi-transparent overlay
        sf::RectangleShape overlay(window.getView().getSize());
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
        
        // Draw menu
    }
    
    void handleEvent(const sf::Event& event) override {
        if (resumePressed) {
            manager->popScene();  // Back to gameplay
        }
        if (quitPressed) {
            auto menu = manager->createScene<MenuScene>();
            manager->changeScene(std::move(menu));  // Clears stack
        }
    }
};
```

## Integration with Main Loop

```cpp
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Game");
    TimeManager time;
    SceneManager scenes;
    
    // Start with main menu
    scenes.changeScene(scenes.createScene<MenuScene>());
    
    while (window.isOpen()) {
        // Time
        time.update();
        float dt = time.getDeltaTime();
        
        // Events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            scenes.handleEvent(event);
        }
        
        // Update
        scenes.update(dt);
        
        // Render
        window.clear();
        scenes.render(window);
        window.display();
    }
    
    return 0;
}
```

## Common Gotchas

### ✗ Don't do heavy work in constructor
```cpp
// Bad
GameplayScene::GameplayScene() {
    texture.loadFromFile("huge.png");  // Blocks!
}

// Good
void GameplayScene::onEnter() {
    texture.loadFromFile("huge.png");  // Loads when needed
}
```

### ✗ Don't store raw Scene pointers
```cpp
// Bad
Scene* myScene = manager.createScene<GameplayScene>().get();
manager.changeScene(std::move(myScene));  // Dangling pointer!

// Good
auto scene = manager.createScene<GameplayScene>();
// Use scene immediately or store as unique_ptr
manager.changeScene(std::move(scene));
```

### ✗ Don't forget to pop overlays
```cpp
// Showing pause menu
manager->pushScene(createScene<PauseScene>());

// Later: going to main menu
// Bad - leaves pause menu on stack!
manager->changeScene(createScene<MenuScene>());

// Good - changeScene clears entire stack
manager->changeScene(createScene<MenuScene>());  // ✓ Correct
```

### ✓ Do use onPause/onResume correctly
```cpp
void onPause() override {
    music.pause();     // ✓ Keep state, just pause
    // Don't clear game state!
}

void onExit() override {
    music.stop();      // ✓ Clean up, scene is done
    saveProgress();
}

void onResume() override {
    music.play();      // ✓ Continue from where we left off
}
```

## Performance Tips

1. **Limit stack depth**: Keep it under 5 scenes typically
2. **Opaque overlays**: Use solid backgrounds to skip rendering lower scenes
3. **Cleanup in onExit()**: Release heavy resources when done
4. **Preload in onEnter()**: Load resources when scene activates
5. **Fast transitions**: Use 0.2-0.3s for responsive feel

## Debug Helpers

```cpp
// Add to SceneManager for debugging
void printStack() const {
    std::cout << "Scene Stack (" << sceneStack.size() << "):\n";
    for (size_t i = 0; i < sceneStack.size(); ++i) {
        std::cout << "  [" << i << "] "
                  << (i == sceneStack.size() - 1 ? "* " : "  ")
                  << typeid(*sceneStack[i]).name() << "\n";
    }
}
```

Use in scenes:
```cpp
void GameplayScene::onEnter() {
    std::cout << "GameplayScene entered\n";
    manager->printStack();  // See full stack
}
```