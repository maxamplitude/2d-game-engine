#pragma once
#include "rendering/texture_atlas.h"
#include <string>
#include <functional>

namespace Engine {

class AnimationController {
public:
    explicit AnimationController(TextureAtlas* atlas);
    ~AnimationController() = default;
    
    // Playback control
    void play(const std::string& animation_name, bool restart = false);
    void stop();
    void pause();
    void resume();
    void reset();  // Reset to frame 0 without stopping
    
    // Update - call every frame
    void update(float dt);
    
    // Current state queries
    bool is_playing() const { return playing && !paused; }
    bool is_paused() const { return paused; }
    bool is_finished() const { return finished; }
    bool has_animation() const { return current_anim != nullptr; }
    
    std::string get_current_animation_name() const { return current_anim_name; }
    int get_current_frame_index() const { return current_frame_index; }
    int get_frame_count() const;
    float get_progress() const;  // 0.0 to 1.0
    
    // Get current frame data for rendering
    const SpriteFrame* get_current_frame() const;
    
    // Playback speed
    void set_speed(float speed) { playback_speed = speed; }
    float get_speed() const { return playback_speed; }
    
    // Callbacks (optional)
    using AnimationCallback = std::function<void()>;
    using FrameCallback = std::function<void(int frame_index)>;
    
    void set_on_animation_end(AnimationCallback callback) { on_animation_end = callback; }
    void set_on_animation_loop(AnimationCallback callback) { on_animation_loop = callback; }
    void set_on_frame_change(FrameCallback callback) { on_frame_change = callback; }
    
    // Clear all callbacks
    void clear_callbacks();
    
private:
    TextureAtlas* atlas;
    const AnimationData* current_anim = nullptr;
    std::string current_anim_name;
    
    int current_frame_index = 0;
    float current_frame_time = 0.0f;
    float playback_speed = 1.0f;
    
    bool playing = false;
    bool paused = false;
    bool finished = false;
    
    // Callbacks
    AnimationCallback on_animation_end;
    AnimationCallback on_animation_loop;
    FrameCallback on_frame_change;
    
    // Helper methods
    float get_current_frame_duration() const;
    void advance_frame();
    void set_frame(int frame_index);
};

} // namespace Engine

