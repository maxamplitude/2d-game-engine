#include "texture_atlas.h"
#include "platform/logging.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Engine {

bool TextureAtlas::loadFromFile(const std::string& texturePath,
                                const std::string& metadataPath) {
    clear();

    if (!texture.loadFromFile(texturePath)) {
        Log::warn("Texture atlas image not loaded ({}), continuing with metadata only", texturePath);
    }

    if (!parseMetadata(metadataPath)) {
        Log::error("Failed to parse metadata: {}", metadataPath);
        return false;
    }

    return true;
}

bool TextureAtlas::parseMetadata(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Log::error("Could not open metadata file: {}", path);
        return false;
    }

    try {
        nlohmann::json json;
        file >> json;

        if (json.contains("frames")) {
            for (const auto& frameJson : json["frames"]) {
                SpriteFrame frame;
                frame.name = frameJson.value("name", "");
                frame.x = frameJson.value("x", 0);
                frame.y = frameJson.value("y", 0);
                frame.w = frameJson.value("w", 0);
                frame.h = frameJson.value("h", 0);

                float originX = frameJson.value("originX", frameJson.value("origin_x", frame.w * 0.5f));
                float originY = frameJson.value("originY", frameJson.value("origin_y", static_cast<float>(frame.h)));
                frame.origin = Vec2(originX, originY);

                frames[frame.name] = frame;
            }
        }

        if (json.contains("animations")) {
            for (const auto& animJson : json["animations"]) {
                AnimationData anim;
                anim.name = animJson.value("name", "");
                anim.frameDuration = animJson.value("frameDuration", 0.1f);
                anim.loop = animJson.value("loop", true);

                if (animJson.contains("frames")) {
                    for (const auto& frameName : animJson["frames"]) {
                        anim.frameNames.push_back(frameName);
                    }
                }

                if (animJson.contains("frameDurations")) {
                    for (const auto& dur : animJson["frameDurations"]) {
                        anim.frameDurations.push_back(dur);
                    }
                }

                bool valid = true;
                for (const auto& frameName : anim.frameNames) {
                    if (frames.find(frameName) == frames.end()) {
                        Log::warn("Animation '{}' references unknown frame '{}'", anim.name, frameName);
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

const SpriteFrame* TextureAtlas::getFrame(const std::string& name) const {
    auto it = frames.find(name);
    return (it != frames.end()) ? &it->second : nullptr;
}

const AnimationData* TextureAtlas::getAnimation(const std::string& name) const {
    auto it = animations.find(name);
    return (it != animations.end()) ? &it->second : nullptr;
}

std::vector<std::string> TextureAtlas::getAnimationNames() const {
    std::vector<std::string> names;
    names.reserve(animations.size());
    for (const auto& [name, _] : animations) {
        names.push_back(name);
    }
    return names;
}

bool TextureAtlas::hasAnimation(const std::string& name) const {
    return animations.find(name) != animations.end();
}

std::vector<std::string> TextureAtlas::getFrameNames() const {
    std::vector<std::string> names;
    names.reserve(frames.size());
    for (const auto& [name, _] : frames) {
        names.push_back(name);
    }
    return names;
}

void TextureAtlas::addFrame(const SpriteFrame& frame) {
    frames[frame.name] = frame;
}

void TextureAtlas::addAnimation(const AnimationData& animation) {
    for (const auto& frameName : animation.frameNames) {
        if (frames.find(frameName) == frames.end()) {
            Log::warn("Animation '{}' references missing frame '{}', skipping", animation.name, frameName);
            return;
        }
    }
    animations[animation.name] = animation;
}

void TextureAtlas::clear() {
    frames.clear();
    animations.clear();
}

} // namespace Engine