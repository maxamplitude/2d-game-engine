# Scene Management System - Integration Guide

## Overview

The scene management system provides a robust, production-ready foundation for managing game states. It handles scene lifecycles, smooth transitions, and scene stacking (for pause menus, overlays, etc.).

## How It Works

### Architecture

```
SceneManager (owns and orchestrates)
    ↓
Scene Stack: [BaseScene, OverlayScene, TopScene]
                                        ↑
                                   (receives events/updates)
```

**Key Concepts:**

1. **Scene**: Abstract base class representing a game state
   - Virtual lifecycle hooks: `onEnter()`, `onExit()`, `onPause()`, `onResume()`
   - Pure virtual methods: `update(dt)`, `render(window)`
   - Optional: `handleEvent(event)`

2. **SceneManager**: Controls scene stack and transitions
   - Owns all scenes (via `unique_ptr`)
   - Delegates events/updates to top scene only
   - Renders all scenes (bottom to top) for transparency
   - Handles transition timing and fade overlays

3. **Scene Stack**: Vector of scenes where:
   - Top scene = active (receives events/updates)
   - Lower scenes = paused (still rendered for effects)
   - Push = pause current, add new on top
   - Pop = remove top, resume previous

### Transition State Machine

```
None → FadingOut → [Execute Scene Operation] → FadingIn → None
```

During transitions:
- Scenes don't receive updates (freeze frame)
- Fade overlay alpha animates 0→255→0
- Scene operations (change/push/pop) execute between fade phases

## CMakeLists.txt Integration

Add to **src/CMakeLists.txt**:

```cmake
set(ENGINE_SOURCES
    # ... existing sources ...
    scene/scene_manager.cpp
)

set(ENGINE_HEADERS
    # ... existing headers ...
    scene/scene.h
    scene/scene_manager.h
)
```

No additional dependencies required - uses only SFML and standard library.

## Common Usage Patterns

### Pattern 1: Simple Scene Transitions

```cpp
SceneManager sceneManager;

// Start with menu
auto menu = sceneManager.createScene<MenuScene>();
sceneManager.changeScene(std::move(menu));

// Later: transition to gameplay with fade
TransitionConfig fade(SceneTransition::Fade, 0.5f);
auto gameplay = sceneManager.createScene<GameplayScene>();
sceneManager.changeScene(std::move(gameplay), fade);
```

### Pattern 2: Pause Menu Overlay

```cpp
// In gameplay scene's event handler:
void GameplayScene::handleEvent(const sf::Event& event) {
    if (event.key.code == sf::Keyboard::Escape) {
        // Push pause menu on top (keeps gameplay visible underneath)
        auto pause = manager->createScene<PauseScene>();
        manager->pushScene(std::move(pause));
    }
}

// In pause menu:
void PauseScene::handleEvent(const sf::Event& event) {
    if (event.key.code == sf::Keyboard::Escape) {
        // Resume gameplay
        manager->popScene();
    }
}
```

### Pattern 3: Multiple Overlays

```cpp
// Stack: [Gameplay] 
manager->pushScene(createScene<InventoryScene>());
// Stack: [Gameplay, Inventory]

manager->pushScene(createScene<ItemDetailScene>());
// Stack: [Gameplay, Inventory, ItemDetail]

// Close detail view
manager->popScene();
// Stack: [Gameplay, Inventory] <- Inventory receives control

// Close inventory
manager->popScene();
// Stack: [Gameplay] <- Gameplay receives control
```

### Pattern 4: Passing Data to Scenes

```cpp
// Scene with constructor parameters
class DialogueScene : public Scene {
public:
    DialogueScene(const std::string& npcName, 
                  const std::vector<std::string>& lines)
        : npcName(npcName), dialogueLines(lines) {}
    // ...
};

// Create with parameters
auto dialogue = sceneManager.createScene<DialogueScene>(
    "Merchant", 
    std::vector<std::string>{"Hello!", "What can I get you?"}
);
manager->pushScene(std::move(dialogue));
```

### Pattern 5: Custom Transition Colors

```cpp
// White fade (useful for dream sequences, flashbacks)
TransitionConfig whiteFade(
    SceneTransition::Fade, 
    0.3f,
    sf::Color::White
);
manager->changeScene(std::move(scene), whiteFade);

// Red fade (for damage/death)
TransitionConfig redFade(
    SceneTransition::Fade, 
    0.2f,
    sf::Color(255, 0, 0)
);
manager->changeScene(std::move(gameOverScene), redFade);
```

## Scene Lifecycle Reference

### Method Call Order

**changeScene(A → B):**
```
1. A->onExit()
2. B->onEnter()
```

**pushScene(B on top of A):**
```
1. A->onPause()
2. B->onEnter()
```

**popScene(remove B, expose A):**
```
1. B->onExit()
2. A->onResume()
```

### When Methods Are Called

| Method | When |
|--------|------|
| `onEnter()` | Scene becomes active (first time or after resume) |
| `onExit()` | Scene is being removed completely |
| `onPause()` | Another scene is pushed on top |
| `onResume()` | Top scene is popped, this becomes active again |
| `update(dt)` | Every frame (only top scene) |
| `render(window)` | Every frame (all scenes in stack) |
| `handleEvent(event)` | Per event (only top scene) |

## Design Decisions & Rationale

### Why unique_ptr Ownership?

```cpp
void changeScene(std::unique_ptr<Scene> newScene);
```

**Pros:**
- Clear ownership semantics (SceneManager owns scenes)
- Automatic cleanup (no manual delete)
- Move-only semantics prevent accidental copies
- Zero runtime overhead

**Alternative considered:** Shared ownership (`shared_ptr`)
- Rejected: Scenes shouldn't be shared between managers
- Rejected: Circular reference risk if scenes reference manager

### Why Virtual Methods vs Callback Lambdas?

```cpp
class Scene {
    virtual void update(float dt) = 0;  // ✓ Used this
    // vs
    // std::function<void(float)> onUpdate;  // ✗ Not used
};
```

**Rationale:**
- Virtual methods are more discoverable (IDE autocomplete)
- No need to wire up callbacks manually
- Slightly better performance (no function pointer indirection)
- Cleaner for large scene hierarchies

**When callbacks make sense:**
- One-off transitions: `scene->onComplete = []() { ... };`
- We still support this via the manager pointer

### Why Scene Stack vs Single Active Scene?

```cpp
std::vector<std::unique_ptr<Scene>> sceneStack;  // ✓ Used this
// vs
// std::unique_ptr<Scene> activeScene;  // ✗ Not used
```

**Rationale:**
- Enables pause menus without destroying game state
- Supports complex UI overlays (inventory → item detail → confirmation)
- Allows dimmed backgrounds (render all scenes)
- Minimal complexity cost (stack is small, <5 scenes typical)

### Why Update Only Top, Render All?

```cpp
void SceneManager::update(float dt) {
    sceneStack.back()->update(dt);  // Only top
}

void SceneManager::render(sf::RenderWindow& window) {
    for (auto& scene : sceneStack) {  // All scenes
        scene->render(window);
    }
}
```

**Rationale:**
- Update top only: Paused scenes shouldn't simulate
- Render all: Enables transparency effects (pause menu over gameplay)
- Performance: Rendering is typically lightweight for UI overlays
- Flexibility: Scenes can use `sf::RenderStates` for alpha blending

**If you need different behavior:**
```cpp
// In PauseScene::render() if you want gameplay hidden:
void render(sf::RenderWindow& window) override {
    // Draw opaque background first
    sf::RectangleShape bg(window.getSize());
    bg.setFillColor(sf::Color::Black);
    window.draw(bg);
    
    // Then draw pause UI
    // ...
}
```

### Why Transition State Machine?

```
None → FadingOut → FadingIn → None
```

**Rationale:**
- Clean separation of fade-out (old scene) vs fade-in (new scene)
- Scene operations happen at perfect midpoint (fully black)
- Prevents visual glitches from immediate scene switches
- Configurable timing per transition

**Alternative considered:** Single fade phase
- Rejected: Would show partial new scene during old scene's fade
- Rejected: Harder to time scene switch precisely

## Integration with Existing Systems

### TimeManager Integration

```cpp
void gameLoop() {
    TimeManager timeManager;
    SceneManager sceneManager;
    
    while (window.isOpen()) {
        timeManager.update();
        float dt = timeManager.getDeltaTime();
        
        sceneManager.update(dt);  // Pass delta time to scenes
        sceneManager.render(window);
    }
}
```

### AnimationController Integration

```cpp
class GameplayScene : public Scene {
private:
    TextureAtlas atlas;
    AnimationController playerAnim;
    
public:
    GameplayScene() : playerAnim(&atlas) {}
    
    void onEnter() override {
        atlas.loadFromFile("player.png", "player.json");
        playerAnim.play("idle");
    }
    
    void update(float dt) override {
        playerAnim.update(dt);
    }
    
    void render(sf::RenderWindow& window) override {
        window.draw(playerAnim.getCurrentSprite());
    }
};
```

### Input System Integration

```cpp
class GameplayScene : public Scene {
private:
    InputMapper input;
    
public:
    void onEnter() override {
        // Bind actions
        input.bindAction("jump", sf::Keyboard::Space);
        input.bindAction("pause", sf::Keyboard::Escape);
    }
    
    void handleEvent(const sf::Event& event) override {
        input.handleEvent(event);
    }
    
    void update(float dt) override {
        if (input.isActionPressed("jump")) {
            // Jump logic
        }
        
        if (input.isActionPressed("pause")) {
            auto pause = manager->createScene<PauseScene>();
            manager->pushScene(std::move(pause));
        }
    }
};
```

## Testing Strategy

### Unit Test Coverage

The scene management system has comprehensive unit tests covering:

1. **Lifecycle correctness**: All hooks called in proper order
2. **Stack operations**: Push/pop/change behavior
3. **Transition timing**: Fade duration and state changes
4. **Event delegation**: Only top scene receives events
5. **Update delegation**: Only top scene receives updates
6. **Render ordering**: All scenes render bottom-to-top
7. **Edge cases**: Null scenes, empty stack operations

Run tests:
```bash
cd build
ctest -R scene  # Run scene tests only
```

### Integration Testing

For integration tests, create concrete test scenes:

```cpp
TEST_CASE("Full game flow simulation", "[scene][integration]") {
    SceneManager manager;
    sf::RenderWindow window(...);
    
    // Menu → Gameplay → Pause → Resume → GameOver
    auto menu = manager.createScene<MenuScene>();
    manager.changeScene(std::move(menu));
    
    // Simulate user starting game
    // ... trigger scene transition
    
    // Simulate opening pause menu
    // ... verify stack depth
    
    // etc.
}
```

## Best Practices

### 1. Resource Management

**Don't load resources in constructor:**
```cpp
// ✗ Bad
class GameplayScene : public Scene {
    GameplayScene() {
        texture.loadFromFile("heavy.png");  // Blocks construction
    }
};

// ✓ Good
class GameplayScene : public Scene {
    void onEnter() override {
        texture.loadFromFile("heavy.png");  // Loads when scene activates
    }
};
```

### 2. Scene Communication

**Use manager pointer for transitions:**
```cpp
// ✓ Good
void GameplayScene::update(float dt) {
    if (playerDied) {
        auto gameOver = manager->createScene<GameOverScene>(score);
        manager->changeScene(std::move(gameOver));
    }
}
```

**For cross-scene data, use scene constructors:**
```cpp
class GameOverScene : public Scene {
    int finalScore;
public:
    GameOverScene(int score) : finalScore(score) {}
};
```

### 3. Cleanup in onExit()

```cpp
void GameplayScene::onExit() override {
    // Stop music
    music.stop();
    
    // Save game state
    saveGame();
    
    // Clear resources if needed
    enemies.clear();
}
```

### 4. Distinguish onPause() vs onExit()

```cpp
void onPause() override {
    // Scene is still in memory, just inactive
    // Keep game state intact
    music.pause();  // Not stop()
}

void onExit() override {
    // Scene is being destroyed
    // Clean up everything
    music.stop();
    saveProgress();
}
```

### 5. Handle Resume State

```cpp
void onResume() override {
    // Returning from pause/overlay
    music.play();
    
    // Re-check game state (time might have passed in overlay)
    refreshUI();
}
```

## Performance Considerations

### Scene Stack Depth

- **Typical**: 1-3 scenes (gameplay + pause + dialog)
- **Safe limit**: < 10 scenes
- **Memory**: Each scene ~few KB unless holding large resources
- **Rendering**: All scenes render every frame (use opaque backgrounds if needed)

### Transition Cost

- **Fade transition**: Negligible (one fullscreen quad)
- **Scene switch**: O(1) operation
- **Stack operations**: O(1) (vector operations on small stack)

### Optimization Tips

```cpp
// If lower scenes don't need rendering:
void PauseScene::render(sf::RenderWindow& window) override {
    // Clear screen = skip rendering lower scenes visually
    window.clear(sf::Color::Black);
    
    // Draw UI
    // ...
}

// Or implement render flag system:
class Scene {
    virtual bool shouldRenderBeneath() const { return false; }
};

// In SceneManager::render():
for (size_t i = 0; i < sceneStack.size(); ++i) {
    if (i < sceneStack.size() - 1 && 
        !sceneStack[i+1]->shouldRenderBeneath()) {
        continue;  // Skip this scene
    }
    sceneStack[i]->render(window);
}
```

## Future Extensions

### Potential Enhancements

1. **Scene Pooling**: Reuse scene instances
```cpp
std::unordered_map<std::string, std::unique_ptr<Scene>> scenePool;
```

2. **Scene Serialization**: Save/load scene state
```cpp
virtual void serialize(json& j) const;
virtual void deserialize(const json& j);
```

3. **Transition Effects**: Custom shaders
```cpp
struct TransitionConfig {
    std::unique_ptr<sf::Shader> customShader;
};
```

4. **Scene History**: Navigate back
```cpp
void previousScene();  // Like browser back button
```

5. **Async Scene Loading**: Background loading
```cpp
void preloadScene<T>(Args&&... args);
```

But for now, the current implementation covers 95% of use cases with minimal complexity.