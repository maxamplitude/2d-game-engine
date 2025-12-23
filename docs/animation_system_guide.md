# Animation System Guide

## Architecture Overview

The animation system consists of three layers:

### Layer 1: TextureAtlas
- **Purpose:** Load and store sprite frame data
- **Responsibilities:** Parse JSON metadata, provide frame/animation lookup
- **Key Features:** UV coordinates, origin points, variable frame durations

### Layer 2: AnimationController  
- **Purpose:** Play animations and manage frame advancement
- **Responsibilities:** Time-based frame updates, playback control, callbacks
- **Key Features:** Loop/one-shot, speed control, progress tracking

### Layer 3: AnimationStateMachine
- **Purpose:** Context-aware animation management
- **Responsibilities:** State transitions, priority system, automatic transitions
- **Key Features:** Priority interruption, pending transitions, predicates

## Quick Start

### 1. Create Sprite Sheet and Metadata

**Sprite sheet layout (power-of-2 dimensions recommended):**
```
player_atlas.png (128x96)
Row 0: idle frames (32x32 each)
Row 1: walk frames (32x32 each)
Row 2: jump frames (32x32 each)
```

**Metadata JSON:**
```json
{
  "texture": "player_atlas.png",
  "texture_width": 128,
  "texture_height": 96,
  "frames": [
    {
      "name": "player_idle_0",
      "x": 0, "y": 0, "w": 32, "h": 32,
      "origin_x": 16, "origin_y": 32
    }
  ],
  "animations": [
    {
      "name": "idle",
      "frames": ["player_idle_0", "player_idle_1"],
      "frame_duration": 0.15,
      "loop": true
    }
  ]
}
```

### 2. Load and Play
```cpp
// Setup
TextureAtlas atlas;
atlas.load_from_file("player.png", "player.json");

AnimationController controller(&atlas);
controller.play("idle");

// Game loop
void update(float dt) {
    controller.update(dt);
    
    const SpriteFrame* frame = controller.get_current_frame();
    // Use frame->uv_rect for rendering
}
```

### 3. Add State Machine (Optional but Recommended)
```cpp
AnimationStateMachine state_machine(&controller);

// Define states
state_machine.add_state("idle", 0);
state_machine.add_state("walk", 1);
state_machine.add_state("jump", 2);
state_machine.add_state("attack", 3);

// Define transitions
state_machine.add_transition("idle", "walk",
    TransitionCondition::Immediate,
    [&]() { return is_moving; });

state_machine.add_transition("walk", "idle",
    TransitionCondition::Immediate,
    [&]() { return !is_moving; });

// Use in game
state_machine.transition_to("idle");
state_machine.update(dt);  // Handles controller.update() internally
```

## Priority System

States have priorities to control interruptibility:
```cpp
state_machine.add_state("idle", 0);      // Always interruptible
state_machine.add_state("walk", 1);      // Medium priority
state_machine.add_state("attack", 3);    // Hard to interrupt
state_machine.add_state("hurt", 4);      // Cannot be interrupted

// Example:
state_machine.transition_to("attack");
state_machine.transition_to("idle");  // Fails, queued as pending

// Animation finishes -> automatically transitions to pending "idle"
```

**Force transition to override priority:**
```cpp
state_machine.transition_to("idle", true);  // Force=true ignores priority
```

## Transition Conditions

### Immediate
Transitions immediately if predicate returns true:
```cpp
state_machine.add_transition("idle", "walk",
    TransitionCondition::Immediate,
    [&]() { return velocity.x != 0; });
```

### OnFinish
Waits for current animation to complete:
```cpp
state_machine.add_transition("attack", "idle",
    TransitionCondition::OnFinish);
```

### CanInterrupt
Checks priority system before transitioning:
```cpp
state_machine.add_transition("walk", "attack",
    TransitionCondition::CanInterrupt,
    [&]() { return attack_button_pressed; });
```

## Callbacks

### AnimationController Callbacks
```cpp
// Triggered when non-looping animation finishes
controller.set_on_animation_end([]() {
    std::cout << "Animation ended\n";
});

// Triggered each time animation loops
controller.set_on_animation_loop([]() {
    play_footstep_sound();
});

// Triggered on frame change
controller.set_on_frame_change([](int frame) {
    if (frame == 3) {
        spawn_attack_hitbox();
    }
});
```

## Performance Characteristics

**Per-entity cost:**
- AnimationController: ~10-20 CPU instructions/frame
- StateMachine: +~50-100 instructions/frame (transitions)
- Total: <0.001ms per entity on modern CPU

**With 1000 animated entities:**
- ~1ms CPU time
- Negligible compared to rendering (5-15ms)

## Best Practices

### 1. Origin Points
Set origin to bottom-center for ground-based characters:
```json
"origin_x": 16,  // width / 2
"origin_y": 32   // height (bottom)
```

This makes sprite positioning consistent across different poses.

### 2. Frame Durations
Use variable frame durations for cinematic effect:
```json
"animations": [{
  "name": "jump",
  "frames": ["jump_0", "jump_1", "jump_2"],
  "frame_durations": [0.05, 0.15, 0.3],
  "loop": false
}]
```
Quick anticipation → medium rise → long fall

### 3. State Priority Ranges
```
0-1:   Movement (idle, walk, run)
2-3:   Actions (jump, dash, slide)
4-5:   Attacks (light, heavy)
6-7:   Reactions (hurt, knockback)
8+:    Cutscene/scripted
```

### 4. Predicate Patterns

**Use member variables, not immediate state:**
```cpp
// BAD - expensive, called every frame
state_machine.add_transition("idle", "jump",
    TransitionCondition::Immediate,
    [&]() { return input_manager.is_pressed(Key::Space); });

// GOOD - check once, store result
bool jump_requested = false;

void handle_input() {
    jump_requested = input.is_pressed(Key::Space);
}

state_machine.add_transition("idle", "jump",
    TransitionCondition::Immediate,
    [&]() { return jump_requested; });
```

### 5. Bidirectional Transitions

Always define both directions:
```cpp
// idle <-> walk
state_machine.add_transition("idle", "walk",
    TransitionCondition::Immediate,
    [&]() { return is_moving; });

state_machine.add_transition("walk", "idle",
    TransitionCondition::Immediate,
    [&]() { return !is_moving; });
```

## Integration with Rendering
```cpp
void render_entity(const Entity& entity) {
    const SpriteFrame* frame = entity.anim_controller.get_current_frame();
    if (!frame) return;
    
    // Build sprite quad
    float w = frame->size.x;
    float h = frame->size.y;
    float ox = frame->origin.x;
    float oy = frame->origin.y;
    
    // UV coordinates (normalized 0-1)
    float u0 = frame->uv_rect.x;
    float v0 = frame->uv_rect.y;
    float u1 = u0 + frame->uv_rect.z;
    float v1 = v0 + frame->uv_rect.w;
    
    // Create quad vertices with proper origin offset
    Vertex vertices[4] = {
        {entity.pos.x - ox,      entity.pos.y - oy,      u0, v0},
        {entity.pos.x + w - ox,  entity.pos.y - oy,      u1, v0},
        {entity.pos.x + w - ox,  entity.pos.y + h - oy,  u1, v1},
        {entity.pos.x - ox,      entity.pos.y + h - oy,  u0, v1}
    };
    
    // Submit to BGFX
    bgfx::setVertexBuffer(0, vertex_buffer);
    bgfx::setTexture(0, sampler, texture_handle);
    bgfx::submit(view_id, program);
}
```

## Common Patterns

### Platformer Character
```cpp
// States
state_machine.add_state("idle", 0);
state_machine.add_state("walk", 1);
state_machine.add_state("run", 1);
state_machine.add_state("jump", 2);
state_machine.add_state("fall", 2);
state_machine.add_state("land", 2);
state_machine.add_state("attack", 3);

// Movement transitions
state_machine.add_transition("idle", "walk", Immediate,
    [&]() { return abs(velocity.x) > 0.1f && abs(velocity.x) < run_threshold; });

state_machine.add_transition("walk", "run", Immediate,
    [&]() { return abs(velocity.x) >= run_threshold; });

// Air transitions
state_machine.add_transition("idle", "jump", Immediate,
    [&]() { return !grounded && velocity.y < 0; });

state_machine.add_transition("jump", "fall", Immediate,
    [&]() { return velocity.y >= 0; });

state_machine.add_transition("fall", "land", Immediate,
    [&]() { return grounded; });

state_machine.add_transition("land", "idle", OnFinish);
```

### Top-Down Character
```cpp
// States (simpler - no gravity)
state_machine.add_state("idle", 0);
state_machine.add_state("walk_n", 1);
state_machine.add_state("walk_s", 1);
state_machine.add_state("walk_e", 1);
state_machine.add_state("walk_w", 1);
state_machine.add_state("attack", 2);

// Directional walking
state_machine.add_transition("idle", "walk_n", Immediate,
    [&]() { return velocity.y < -0.1f; });

state_machine.add_transition("walk_n", "idle", Immediate,
    [&]() { return abs(velocity.y) <= 0.1f; });
```

## Debugging
```cpp
// Log state changes
state_machine.transition_to("walk");
std::cout << "State: " << state_machine.get_current_state() << "\n";
std::cout << "Priority: " << state_machine.get_current_priority() << "\n";

// Check pending transitions
if (state_machine.has_pending_transition()) {
    std::cout << "Pending: " << state_machine.get_pending_state() << "\n";
}

// Debug animation playback
std::cout << "Animation: " << controller.get_current_animation_name() << "\n";
std::cout << "Frame: " << controller.get_current_frame_index() 
          << "/" << controller.get_frame_count() << "\n";
std::cout << "Progress: " << (controller.get_progress() * 100) << "%\n";
```

## Next Steps

1. **Input System:** Add proper input handling with buffering
2. **Physics:** Integrate with RigidBody for movement
3. **Sprite Flipping:** Add horizontal flip for facing direction
4. **Animation Events:** Trigger gameplay effects at specific frames
5. **Blend Trees:** Smooth transitions between animations (advanced)