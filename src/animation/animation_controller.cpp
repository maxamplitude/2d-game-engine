#include "animation_controller.h"
#include <algorithm>
#include <stdexcept>

namespace Engine {

AnimationController::AnimationController(TextureAtlas* atlas) 
    : atlas(atlas) {
    if (!atlas) {
        throw std::invalid_argument("TextureAtlas cannot be null");
    }
}

void AnimationController::play(const std::string& animation_name, bool restart) {
    const AnimationData* anim = atlas->get_animation(animation_name);
    if (!anim) {
        // Animation not found - stop current playback
        stop();
        return;
    }
    
    const AnimationData* previous_anim = current_anim;

    // Already playing this animation and not restarting?
    if (previous_anim == anim && playing && !restart) {
        return;
    }
    
    // Start playing
    current_anim = anim;
    current_anim_name = animation_name;
    
    if (restart || previous_anim != anim) {
        current_frame_index = 0;
        current_frame_time = 0.0f;
    }
    
    playing = true;
    paused = false;
    finished = false;
}

void AnimationController::stop() {
    playing = false;
    paused = false;
    finished = true;
    current_anim = nullptr;
    current_anim_name.clear();
    current_frame_index = 0;
    current_frame_time = 0.0f;
}

void AnimationController::pause() {
    if (playing) {
        paused = true;
    }
}

void AnimationController::resume() {
    if (playing) {
        paused = false;
    }
}

void AnimationController::reset() {
    current_frame_index = 0;
    current_frame_time = 0.0f;
    finished = false;
}

void AnimationController::update(float dt) {
    if (!playing || paused || !current_anim || finished) {
        return;
    }
    
    // Apply playback speed
    dt *= playback_speed;
    current_frame_time += dt;
    
    // Advance frames as needed
    float frame_duration = get_current_frame_duration();
    const float epsilon = 1e-6f;
    
    // Advance through multiple frames when dt is large; epsilon avoids precision stalls
    int safety = 0;
    const int max_steps = 2048;
    while (playing && safety < max_steps) {
        // Protect against zero/negative durations to avoid infinite loops
        if (frame_duration <= 0.0f) {
            advance_frame();
            frame_duration = get_current_frame_duration();
            ++safety;
            continue;
        }

        if (current_frame_time + epsilon < frame_duration) {
            break;
        }

        current_frame_time -= frame_duration;
        advance_frame();
        ++safety;
        
        // Get new frame duration (might be different for next frame)
        frame_duration = get_current_frame_duration();
    }
}

void AnimationController::advance_frame() {
    if (!current_anim) return;
    
    int previous_frame = current_frame_index;
    current_frame_index++;
    
    // Check if we've reached the end
    if (current_frame_index >= static_cast<int>(current_anim->get_frame_count())) {
        if (current_anim->loop) {
            // Loop back to beginning
            current_frame_index = 0;
            
            // Trigger loop callback
            if (on_animation_loop) {
                on_animation_loop();
            }
        } else {
            // Non-looping: stay on last frame and mark finished
            current_frame_index = static_cast<int>(current_anim->get_frame_count()) - 1;
            playing = false;
            finished = true;
            
            // Trigger end callback
            if (on_animation_end) {
                on_animation_end();
            }
            
            return;
        }
    }
    
    // Trigger frame change callback
    if (on_frame_change && current_frame_index != previous_frame) {
        on_frame_change(current_frame_index);
    }
}

void AnimationController::set_frame(int frame_index) {
    if (!current_anim) return;
    
    int frame_count = static_cast<int>(current_anim->get_frame_count());
    current_frame_index = std::clamp(frame_index, 0, frame_count - 1);
    current_frame_time = 0.0f;
}

float AnimationController::get_current_frame_duration() const {
    if (!current_anim) return 0.0f;
    
    return current_anim->get_duration(current_frame_index);
}

const SpriteFrame* AnimationController::get_current_frame() const {
    if (!current_anim) return nullptr;
    
    if (current_frame_index >= static_cast<int>(current_anim->frame_names.size())) {
        return nullptr;
    }
    
    const std::string& frame_name = current_anim->frame_names[current_frame_index];
    return atlas->get_frame(frame_name);
}

int AnimationController::get_frame_count() const {
    return current_anim ? static_cast<int>(current_anim->get_frame_count()) : 0;
}

float AnimationController::get_progress() const {
    if (!current_anim || current_anim->get_frame_count() == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(current_frame_index) / 
           static_cast<float>(current_anim->get_frame_count() - 1);
}

void AnimationController::clear_callbacks() {
    on_animation_end = nullptr;
    on_animation_loop = nullptr;
    on_frame_change = nullptr;
}

} // namespace Engine

