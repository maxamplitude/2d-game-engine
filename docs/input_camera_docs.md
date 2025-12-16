# Input and Camera Systems Documentation

## Overview

This document covers the **Input Manager** and **Camera System** - two essential systems for creating responsive, polished games.

## Input System

### Features

The Input System provides sophisticated input handling beyond simple key polling:

1. **State Tracking**: Distinguishes between "just pressed", "held", and "just released"
2. **Action Mapping**: Bind actions to keys or mouse buttons (rebindable controls)
3. **Input Buffering**: Critical for platformer feel - buffers inputs for a short time window
4. **Callbacks**: Execute code automatically when actions are pressed

### Basic Usage

```cpp
#include "input/input_manager.h"

InputManager input;

// In event loop (before processing events)
input.beginFrame();

// Handle events
while (window.pollEvent(event)) {
    if (event.type == sf::Event::KeyPressed) {
        input.handleKeyPressed(event.key.code);
    }
    if (event.type == sf::Event::KeyReleased) {
        input.handleKeyReleased(event.key.code);
    }
}

// Update (after event loop)
input.update(deltaTime);

// Query input states
if (input.isKeyPressed(sf::Keyboard::Space)) {
    // Space just pressed THIS frame
}

if (input.isKeyDown(sf::Keyboard::A)) {
    // A is currently held
}

if (input.isKeyReleased(sf::Keyboard::Space)) {
    // Space just released THIS frame
}
```

### Action Mapping

Action mapping allows you to decouple game logic from specific keys:

```cpp
// Map actions to keys
input.mapAction("jump", sf::Keyboard::Space);
input.mapAction("attack", sf::Mouse::Left);
input.mapAction("move_left", sf::Keyboard::A);

// Use actions in game logic
if (input.isActionPressed("jump")) {
    player.jump();
}

if (input.isActionActive("move_left")) {
    player.moveLeft();
}

// Rebind at runtime
input.mapAction("jump", sf::Keyboard::W);  // Now W jumps
```

### Input Buffering (Jump Buffering)

Input buffering is crucial for platformers. It allows inputs pressed slightly before they should take effect to still register:

```cpp
// When player presses jump in the air
if (input.isActionPressed("jump") && !player.isGrounded()) {
    // Buffer the jump for 150ms
    input.bufferAction("jump", 0.15f);
}

// When player lands
if (player.justLanded()) {
    // Check if there's a buffered jump
    if (input.consumeBufferedAction("jump")) {
        player.jump();
        std::cout << "Buffered jump executed!\n";
    }
}
```

**Why this matters**: Players pressing jump slightly before landing will still jump. This makes controls feel more responsive and forgiving.

### Callbacks

Execute code automatically when actions are pressed:

```cpp
input.setActionPressedCallback("pause", []() {
    game.togglePause();
});

input.setActionPressedCallback("screenshot", []() {
    takeScreenshot();
});
```

### Key vs Action: When to Use Each

- **Raw Keys**: Use for debug commands, UI navigation, or one-off features
  ```cpp
  if (input.isKeyPressed(sf::Keyboard::F12)) {
      takeScreenshot();
  }
  ```

- **Actions**: Use for all gameplay mechanics (allows rebinding)
  ```cpp
  if (input.isActionActive("move_right")) {
      player.moveRight();
  }
  ```

## Camera System

### Features

1. **Smooth Following**: Camera smoothly follows a target with configurable lerp speed
2. **Deadzone Following**: Camera only moves when target exits a deadzone (like Metroidvania games)
3. **Screen Shake**: Trauma-based screen shake system (popularized by game juice talks)
4. **Smooth Zoom**: Zoom transitions with easing
5. **Bounds**: Constrain camera within level boundaries
6. **Coordinate Conversion**: Convert between world and screen space

### Basic Usage

```cpp
#include "rendering/camera.h"

// Create camera at world position, with view size
Camera camera({400.0f, 300.0f}, {800.0f, 600.0f});

// Update each frame
camera.update(deltaTime);

// Apply to SFML window
window.setView(camera.toSFMLView());
```

### Following Targets

```cpp
sf::Vector2f playerPos(100.0f, 200.0f);

// Make camera follow player
camera.setTarget(&playerPos);

// Instant following (camera snaps to target)
camera.setFollowMode(CameraFollowMode::Instant);

// Smooth following (camera lerps toward target)
camera.setFollowMode(CameraFollowMode::Smooth);
camera.setFollowSpeed(5.0f);  // Higher = faster following

// Deadzone following (camera only moves when target exits zone)
camera.setFollowMode(CameraFollowMode::Deadzone);
camera.setDeadzone({-50.0f, -50.0f, 100.0f, 100.0f});  // 100x100 deadzone
```

### Screen Shake

The camera uses a **trauma-based** screen shake system:

```cpp
// Add trauma (0.0 to 1.0)
camera.addTrauma(0.8f);  // Strong shake

// Trauma decays over time automatically
// Shake intensity = trauma^2 (nice falloff curve)

// Customize shake behavior
camera.setShakeIntensity(50.0f);  // Max offset in pixels
camera.setTraumaDecay(1.5f);      // How fast trauma decays
```

**When to use**:
- Player takes damage: `camera.addTrauma(0.5f)`
- Explosion nearby: `camera.addTrauma(0.8f)`
- Player lands: `camera.addTrauma(0.2f)`
- Boss roars: `camera.addTrauma(0.6f)`

### Smooth Zoom

```cpp
// Instant zoom
camera.setZoom(2.0f);

// Smooth zoom transition
camera.zoomTo(1.5f, 0.5f);  // Zoom to 1.5x over 0.5 seconds

// Zoom by factor
camera.zoomBy(0.5f);  // Zoom in (multiply current zoom by 0.5)
```

### Camera Bounds

Prevent camera from showing areas outside your level:

```cpp
// Set world bounds (x, y, width, height)
camera.setBounds({0.0f, 0.0f, 2000.0f, 1500.0f});

// Camera will automatically stay within these bounds
camera.update(deltaTime);

// Remove bounds
camera.clearBounds();
```

### Visibility Testing

Optimize rendering by only drawing visible objects:

```cpp
sf::FloatRect enemyBounds(enemy.x, enemy.y, 32.0f, 32.0f);

if (camera.isVisible(enemyBounds)) {
    window.draw(enemy.sprite);
}
```

### Coordinate Conversion

```cpp
sf::Vector2u windowSize = window.getSize();

// Screen to world (e.g., for mouse clicks)
sf::Vector2f mouseScreen = sf::Mouse::getPosition(window);
sf::Vector2f mouseWorld = camera.screenToWorld(mouseScreen, windowSize);

// World to screen (e.g., for UI positioning)
sf::Vector2f worldPos(100.0f, 200.0f);
sf::Vector2f screenPos = camera.worldToScreen(worldPos, windowSize);
```

## Integration Example

Here's how both systems work together:

```cpp
InputManager input;
Camera camera({400.0f, 300.0f}, {800.0f, 600.0f});
sf::Vector2f playerPos(400.0f, 300.0f);

// Setup
input.mapAction("jump", sf::Keyboard::Space);
camera.setTarget(&playerPos);
camera.setFollowMode(CameraFollowMode::Smooth);
camera.setFollowSpeed(5.0f);

// Game loop
while (running) {
    // 1. Input handling
    input.beginFrame();
    handleEvents();  // Call input.handleKeyPressed(), etc.
    input.update(dt);
    
    // 2. Game logic
    if (input.isActionPressed("jump") && player.isGrounded()) {
        player.jump();
        camera.addTrauma(0.3f);  // Shake on jump
    }
    
    // 3. Update systems
    camera.update(dt);
    
    // 4. Render
    window.setView(camera.toSFMLView());
    drawGame();
}
```

## Best Practices

### Input System

1. **Always call in order**: `beginFrame()` → handle events → `update(dt)`
2. **Use actions for gameplay**: Makes rebinding easy
3. **Use buffering for time-sensitive inputs**: Especially jumps in platformers
4. **Test different buffer times**: 0.1-0.15 seconds is typical for jumps

### Camera System

1. **Smooth following > instant**: Usually feels better (try both!)
2. **Don't overuse shake**: Reserve for impactful moments
3. **Set camera bounds**: Prevents showing outside world
4. **Test follow speed**: Too fast = jittery, too slow = laggy
5. **Deadzone for top-down games**: Great for Zelda-style cameras

## Testing

Both systems have comprehensive unit tests:

```bash
# Build and run tests
cd build
cmake ..
make
ctest

# Or run specific tests
./EngineTests "[input]"
./EngineTests "[camera]"
```

## Common Patterns

### Platformer Controls

```cpp
// Jump buffering
input.setActionPressedCallback("jump", [&]() {
    if (!player.isGrounded()) {
        input.bufferAction("jump", 0.15f);
    }
});

if (player.justLanded() && input.consumeBufferedAction("jump")) {
    player.jump();
}

// Camera shake on impact
if (player.justLanded() && player.fallSpeed > threshold) {
    float trauma = std::min(player.fallSpeed / maxSpeed, 1.0f);
    camera.addTrauma(trauma * 0.5f);
}
```

### Top-Down Game

```cpp
// Deadzone camera
camera.setFollowMode(CameraFollowMode::Deadzone);
camera.setDeadzone({-100.0f, -100.0f, 200.0f, 200.0f});

// Zoom on aiming
if (input.isActionActive("aim")) {
    camera.zoomTo(0.7f, 0.2f);  // Zoom in
} else {
    camera.zoomTo(1.0f, 0.2f);  // Zoom out
}
```

### Combat Feedback

```cpp
// Different shake intensities
void onPlayerHit() {
    camera.addTrauma(0.4f);
}

void onExplosion(float distance) {
    float trauma = 1.0f - (distance / maxRange);
    camera.addTrauma(trauma * 0.8f);
}

void onBossRoar() {
    camera.addTrauma(1.0f);  // Maximum shake
}
```

## Performance Notes

- Input system is lightweight (hash map lookups)
- Camera shake uses pre-seeded RNG (fast)
- Coordinate conversions are simple math (no matrix inversion)
- Visibility testing is just rectangle intersection

## Future Enhancements

Possible additions:

- **Input**: Gamepad support, input recording/playback, input combos
- **Camera**: Look-ahead following, camera zones, cinematic camera paths
- **Both**: Save/load settings, runtime configuration UI

---

For more examples, see `examples/input_camera_demo.cpp`.