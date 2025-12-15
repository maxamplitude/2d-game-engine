#include "texture_atlas.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace Engine {

bool TextureAtlas::loadFromFile(const std::string& texturePath, 
                                const std::string& metadataPath) {
    clear();
    
    // Load texture
    if (!texture.loadFromFile(texturePath)) {
        std::cerr << "Failed to load texture: " << texturePath << std::endl;
        return false;
    }
    
    texture.setSmooth(false);  // Pixel-perfect rendering
    
    // Parse metadata
    if (!parseMetadata(metadataPath)) {
        std::cerr << "Failed to parse metadata: " << metadataPath << std::endl;
        return false;
    }
    
    return true;
}

bool TextureAtlas::parseMetadata(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open metadata file: " << path << std::endl;
        return false;
    }
    
    try {
        nlohmann::json json;
        file >> json;
        
        // Parse frames
        if (json.contains("frames")) {
            for (const auto& frameJson : json["frames"]) {
                SpriteFrame frame;
                frame.name = frameJson["name"].get<std::string>();
                
                // Required fields
                frame.rect.left = frameJson["x"].get<int>();
                frame.rect.top = frameJson["y"].get<int>();
                frame.rect.width = frameJson["w"].get<int>();
                frame.rect.height = frameJson["h"].get<int>();
                
                // Optional origin (defaults to bottom-center)
                if (frameJson.contains("originX") && frameJson.contains("originY")) {
                    frame.origin.x = frameJson["originX"].get<float>();
                    frame.origin.y = frameJson["originY"].get<float>();
                } else {
                    // Default: bottom-center (good for platformer characters)
                    frame.origin.x = frame.rect.width / 2.0f;
                    frame.origin.y = static_cast<float>(frame.rect.height);
                }
                
                frames[frame.name] = frame;
            }
        }
        
        // Parse animations
        if (json.contains("animations")) {
            for (const auto& animJson : json["animations"]) {
                AnimationData anim;
                anim.name = animJson["name"].get<std::string>();
                anim.frameNames = animJson["frames"].get<std::vector<std::string>>();
                
                // Optional fields
                anim.frameDuration = animJson.value("frameDuration", 0.1f);
                anim.loop = animJson.value("loop", true);
                
                // Per-frame durations (optional)
                if (animJson.contains("frameDurations")) {
                    anim.frameDurations = animJson["frameDurations"]
                        .get<std::vector<float>>();
                }
                
                // Validate: all frame names must exist
                bool valid = true;
                for (const auto& frameName : anim.frameNames) {
                    if (frames.find(frameName) == frames.end()) {
                        std::cerr << "Animation '" << anim.name 
                                  << "' references unknown frame: " << frameName << std::endl;
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
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

const SpriteFrame* TextureAtlas::getFrame(const std::string& name) const {
    auto it = frames.find(name);
    return (it != frames.end()) ? &it->second : nullptr;
}

sf::Sprite TextureAtlas::createSprite(const std::string& frameName) const {
    const SpriteFrame* frame = getFrame(frameName);
    
    sf::Sprite sprite;
    if (frame) {
        sprite.setTexture(texture);
        sprite.setTextureRect(frame->rect);
        sprite.setOrigin(frame->origin);
    }
    
    return sprite;
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
    // Validate frame names exist
    for (const auto& frameName : animation.frameNames) {
        if (frames.find(frameName) == frames.end()) {
            std::cerr << "Warning: Animation '" << animation.name 
                      << "' references unknown frame: " << frameName << std::endl;
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