#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace Engine {

// Forward declare for BGFX handle type (future use)
namespace bgfx { struct TextureHandle; }

struct SpriteFrame {
    std::string name;
    
    // UV coordinates in texture (normalized 0.0 to 1.0)
    glm::vec4 uv_rect{0.0f};  // (u, v, width, height)
    
    // Pixel coordinates in texture (for reference/debugging)
    glm::ivec4 pixel_rect{0}; // (x, y, width, height)
    
    // Origin/pivot point in pixels (relative to frame)
    glm::vec2 origin{0.0f, 0.0f};
    
    // Size in pixels
    glm::vec2 size{0.0f, 0.0f};
    
    SpriteFrame() = default;
    SpriteFrame(const std::string& n, const glm::ivec4& rect, const glm::vec2& o = glm::vec2(0.0f))
        : name(n), pixel_rect(rect), origin(o), size(static_cast<float>(rect.z), static_cast<float>(rect.w)) {}
};

struct AnimationData {
    std::string name;
    std::vector<std::string> frame_names;
    float frame_duration = 0.1f;           // Default duration per frame
    std::vector<float> frame_durations;    // Optional: per-frame durations
    bool loop = true;
    
    AnimationData() = default;
    AnimationData(const std::string& n, const std::vector<std::string>& frames, 
                  float duration = 0.1f, bool l = true)
        : name(n), frame_names(frames), frame_duration(duration), loop(l) {}
    
    // Get duration for specific frame
    float get_duration(size_t frame_index) const {
        if (!frame_durations.empty() && frame_index < frame_durations.size()) {
            return frame_durations[frame_index];
        }
        return frame_duration;
    }
    
    size_t get_frame_count() const { return frame_names.size(); }
};

class TextureAtlas {
public:
    TextureAtlas() = default;
    ~TextureAtlas() = default;
    
    // Loading
    bool load_from_file(const std::string& texture_path,
                        const std::string& metadata_path);
    
    // Direct frame access
    const SpriteFrame* get_frame(const std::string& name) const;
    
    // Animation access
    const AnimationData* get_animation(const std::string& name) const;
    std::vector<std::string> get_animation_names() const;
    bool has_animation(const std::string& name) const;
    
    // Texture info
    uint16_t get_texture_width() const { return texture_width; }
    uint16_t get_texture_height() const { return texture_height; }
    
    // Metadata queries
    size_t get_frame_count() const { return frames.size(); }
    size_t get_animation_count() const { return animations.size(); }
    std::vector<std::string> get_frame_names() const;
    
    // Manual frame/animation addition (for programmatic creation)
    void add_frame(const SpriteFrame& frame);
    void add_animation(const AnimationData& animation);
    
    // Clear all data
    void clear();
    
private:
    std::unordered_map<std::string, SpriteFrame> frames;
    std::unordered_map<std::string, AnimationData> animations;
    
    uint16_t texture_width = 0;
    uint16_t texture_height = 0;
    
    bool parse_metadata(const std::string& path);
    void calculate_uv_coords();
};

} // namespace Engine