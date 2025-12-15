#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Engine {

// Single sprite frame definition
struct SpriteFrame {
    std::string name;
    sf::IntRect rect;           // Position in texture
    sf::Vector2f origin{0, 0};  // Pivot point (for rotation/flipping)
    
    SpriteFrame() = default;
    SpriteFrame(const std::string& n, const sf::IntRect& r, const sf::Vector2f& o = {0, 0})
        : name(n), rect(r), origin(o) {}
};

// Animation sequence definition
struct AnimationData {
    std::string name;
    std::vector<std::string> frameNames;
    float frameDuration = 0.1f;           // Default duration per frame
    std::vector<float> frameDurations;    // Optional: per-frame durations
    bool loop = true;
    
    AnimationData() = default;
    AnimationData(const std::string& n, const std::vector<std::string>& frames, 
                  float duration = 0.1f, bool l = true)
        : name(n), frameNames(frames), frameDuration(duration), loop(l) {}
    
    // Get duration for specific frame
    float getDuration(size_t frameIndex) const {
        if (!frameDurations.empty() && frameIndex < frameDurations.size()) {
            return frameDurations[frameIndex];
        }
        return frameDuration;
    }
    
    size_t getFrameCount() const { return frameNames.size(); }
};

class TextureAtlas {
public:
    TextureAtlas() = default;
    ~TextureAtlas() = default;
    
    // Loading
    bool loadFromFile(const std::string& texturePath, 
                     const std::string& metadataPath);
    
    // Direct sprite access
    const SpriteFrame* getFrame(const std::string& name) const;
    sf::Sprite createSprite(const std::string& frameName) const;
    
    // Animation access
    const AnimationData* getAnimation(const std::string& name) const;
    std::vector<std::string> getAnimationNames() const;
    bool hasAnimation(const std::string& name) const;
    
    // Texture access
    const sf::Texture& getTexture() const { return texture; }
    
    // Metadata queries
    size_t getFrameCount() const { return frames.size(); }
    size_t getAnimationCount() const { return animations.size(); }
    std::vector<std::string> getFrameNames() const;
    
    // Manual frame/animation addition (for programmatic creation)
    void addFrame(const SpriteFrame& frame);
    void addAnimation(const AnimationData& animation);
    
    // Clear all data
    void clear();
    
private:
    sf::Texture texture;
    std::unordered_map<std::string, SpriteFrame> frames;
    std::unordered_map<std::string, AnimationData> animations;
    
    bool parseMetadata(const std::string& path);
};

} // namespace Engine