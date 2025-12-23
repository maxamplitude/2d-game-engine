#include "texture_atlas.h"
#include "platform/logging.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace Engine {

bool TextureAtlas::load_from_file(const std::string& texture_path,
                                  const std::string& metadata_path) {
    clear();

    // Texture loading is handled elsewhere in the BGFX pipeline; keep path for future use.
    (void)texture_path;

    if (!parse_metadata(metadata_path)) {
        Log::error("Failed to parse metadata: {}", metadata_path);
        return false;
    }

    calculate_uv_coords();
    return true;
}

bool TextureAtlas::parse_metadata(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Log::error("Could not open metadata file: {}", path);
        return false;
    }

    try {
        nlohmann::json json;
        file >> json;

        // Texture dimensions (required for UV computation)
        texture_width = json.value("texture_width", json.value("textureWidth", static_cast<uint16_t>(0)));
        texture_height = json.value("texture_height", json.value("textureHeight", static_cast<uint16_t>(0)));

        int max_x = 0;
        int max_y = 0;

        if (json.contains("frames")) {
            for (const auto& frame_json : json["frames"]) {
                SpriteFrame frame;
                frame.name = frame_json.value("name", "");

                int x = frame_json.value("x", 0);
                int y = frame_json.value("y", 0);
                int w = frame_json.value("w", 0);
                int h = frame_json.value("h", 0);
                frame.pixel_rect = glm::ivec4(x, y, w, h);
                frame.size = glm::vec2(static_cast<float>(w), static_cast<float>(h));

                float origin_x = frame_json.value("origin_x",
                    frame_json.value("originX", static_cast<float>(w) * 0.5f));
                float origin_y = frame_json.value("origin_y",
                    frame_json.value("originY", static_cast<float>(h)));
                frame.origin = glm::vec2(origin_x, origin_y);

                max_x = std::max(max_x, x + w);
                max_y = std::max(max_y, y + h);

                frames[frame.name] = frame;
            }
        }

        if (texture_width == 0 || texture_height == 0) {
            texture_width = static_cast<uint16_t>(max_x);
            texture_height = static_cast<uint16_t>(max_y);
            if (texture_width == 0 || texture_height == 0) {
                Log::error("Metadata missing texture dimensions and unable to infer from frames");
                return false;
            }
            Log::warn("Texture dimensions inferred from frames ({}x{}). Please specify 'texture_width' and 'texture_height' in metadata.",
                      texture_width, texture_height);
        }

        if (json.contains("animations")) {
            for (const auto& anim_json : json["animations"]) {
                AnimationData anim;
                anim.name = anim_json.value("name", "");
                anim.frame_duration = anim_json.value("frame_duration",
                    anim_json.value("frameDuration", 0.1f));
                anim.loop = anim_json.value("loop", true);

                if (anim_json.contains("frames")) {
                    for (const auto& frame_name : anim_json["frames"]) {
                        anim.frame_names.push_back(frame_name.get<std::string>());
                    }
                }

                if (anim_json.contains("frame_durations")) {
                    for (const auto& dur : anim_json["frame_durations"]) {
                        anim.frame_durations.push_back(dur.get<float>());
                    }
                } else if (anim_json.contains("frameDurations")) {
                    for (const auto& dur : anim_json["frameDurations"]) {
                        anim.frame_durations.push_back(dur.get<float>());
                    }
                }

                bool valid = true;
                for (const auto& frame_name : anim.frame_names) {
                    if (frames.find(frame_name) == frames.end()) {
                        Log::warn("Animation '{}' references unknown frame '{}'", anim.name, frame_name);
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    animations[anim.name] = anim;
                }
            }
        }

        return true;
    } catch (const nlohmann::json::exception& e) {
        Log::error("JSON parsing error: {}", e.what());
        return false;
    }
}

void TextureAtlas::calculate_uv_coords() {
    if (texture_width == 0 || texture_height == 0) {
        return;
    }

    const float inv_width = 1.0f / static_cast<float>(texture_width);
    const float inv_height = 1.0f / static_cast<float>(texture_height);

    for (auto& [_, frame] : frames) {
        frame.uv_rect.x = frame.pixel_rect.x * inv_width;
        frame.uv_rect.y = frame.pixel_rect.y * inv_height;
        frame.uv_rect.z = frame.pixel_rect.z * inv_width;
        frame.uv_rect.w = frame.pixel_rect.w * inv_height;
    }
}

const SpriteFrame* TextureAtlas::get_frame(const std::string& name) const {
    auto it = frames.find(name);
    return (it != frames.end()) ? &it->second : nullptr;
}

const AnimationData* TextureAtlas::get_animation(const std::string& name) const {
    auto it = animations.find(name);
    return (it != animations.end()) ? &it->second : nullptr;
}

std::vector<std::string> TextureAtlas::get_animation_names() const {
    std::vector<std::string> names;
    names.reserve(animations.size());
    for (const auto& [name, _] : animations) {
        names.push_back(name);
    }
    return names;
}

bool TextureAtlas::has_animation(const std::string& name) const {
    return animations.find(name) != animations.end();
}

std::vector<std::string> TextureAtlas::get_frame_names() const {
    std::vector<std::string> names;
    names.reserve(frames.size());
    for (const auto& [name, _] : frames) {
        names.push_back(name);
    }
    return names;
}

void TextureAtlas::add_frame(const SpriteFrame& frame) {
    frames[frame.name] = frame;
    if (texture_width != 0 && texture_height != 0) {
        auto& stored = frames[frame.name];
        stored.size = glm::vec2(static_cast<float>(stored.pixel_rect.z),
                                static_cast<float>(stored.pixel_rect.w));
        stored.uv_rect.x = stored.pixel_rect.x / static_cast<float>(texture_width);
        stored.uv_rect.y = stored.pixel_rect.y / static_cast<float>(texture_height);
        stored.uv_rect.z = stored.pixel_rect.z / static_cast<float>(texture_width);
        stored.uv_rect.w = stored.pixel_rect.w / static_cast<float>(texture_height);
    }
}

void TextureAtlas::add_animation(const AnimationData& animation) {
    for (const auto& frame_name : animation.frame_names) {
        if (frames.find(frame_name) == frames.end()) {
            Log::warn("Animation '{}' references missing frame '{}', skipping", animation.name, frame_name);
            return;
        }
    }
    animations[animation.name] = animation;
}

void TextureAtlas::clear() {
    frames.clear();
    animations.clear();
    texture_width = 0;
    texture_height = 0;
}

} // namespace Engine