#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "rendering/texture_atlas.h"
#include <fstream>

using namespace Engine;

// Test fixture - creates temporary test files
class TextureAtlasFixture {
public:
    TextureAtlasFixture() {
        createTestTexture();
        createTestMetadata();
    }
    
    ~TextureAtlasFixture() {
        // Cleanup
        std::remove("test_texture.png");
        std::remove("test_metadata.json");
    }
    
private:
    void createTestTexture() {
        // Create minimal 64x64 PNG (white square)
        sf::Image image;
        image.create(64, 64, sf::Color::White);
        image.saveToFile("test_texture.png");
    }
    
    void createTestMetadata() {
        std::ofstream file("test_metadata.json");
        file << R"({
            "texture": "test_texture.png",
            "frames": [
                {"name": "frame_0", "x": 0, "y": 0, "w": 16, "h": 16},
                {"name": "frame_1", "x": 16, "y": 0, "w": 16, "h": 16},
                {"name": "frame_2", "x": 32, "y": 0, "w": 16, "h": 16},
                {"name": "frame_3", "x": 48, "y": 0, "w": 16, "h": 16}
            ],
            "animations": [
                {
                    "name": "test_anim",
                    "frames": ["frame_0", "frame_1", "frame_2"],
                    "frameDuration": 0.1,
                    "loop": true
                },
                {
                    "name": "no_loop",
                    "frames": ["frame_0", "frame_1"],
                    "frameDuration": 0.2,
                    "loop": false
                }
            ]
        })";
        file.close();
    }
};

TEST_CASE("TextureAtlas loads from file", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    
    bool loaded = atlas.loadFromFile("test_texture.png", "test_metadata.json");
    
    REQUIRE(loaded);
    REQUIRE(atlas.getFrameCount() == 4);
    REQUIRE(atlas.getAnimationCount() == 2);
}

TEST_CASE("TextureAtlas retrieves frames", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.loadFromFile("test_texture.png", "test_metadata.json");
    
    SECTION("Existing frame") {
        const SpriteFrame* frame = atlas.getFrame("frame_0");
        REQUIRE(frame != nullptr);
        REQUIRE(frame->name == "frame_0");
        REQUIRE(frame->rect.left == 0);
        REQUIRE(frame->rect.top == 0);
        REQUIRE(frame->rect.width == 16);
        REQUIRE(frame->rect.height == 16);
    }
    
    SECTION("Non-existent frame") {
        const SpriteFrame* frame = atlas.getFrame("does_not_exist");
        REQUIRE(frame == nullptr);
    }
}

TEST_CASE("TextureAtlas creates sprites", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.loadFromFile("test_texture.png", "test_metadata.json");
    
    sf::Sprite sprite = atlas.createSprite("frame_1");
    
    REQUIRE(sprite.getTexture() != nullptr);
    REQUIRE(sprite.getTextureRect().left == 16);
    REQUIRE(sprite.getTextureRect().top == 0);
    REQUIRE(sprite.getTextureRect().width == 16);
    REQUIRE(sprite.getTextureRect().height == 16);
}

TEST_CASE("TextureAtlas retrieves animations", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.loadFromFile("test_texture.png", "test_metadata.json");
    
    SECTION("Existing animation") {
        const AnimationData* anim = atlas.getAnimation("test_anim");
        REQUIRE(anim != nullptr);
        REQUIRE(anim->name == "test_anim");
        REQUIRE(anim->getFrameCount() == 3);
        REQUIRE(anim->frameDuration == 0.1f);
        REQUIRE(anim->loop == true);
    }
    
    SECTION("Animation frame names") {
        const AnimationData* anim = atlas.getAnimation("test_anim");
        REQUIRE(anim->frameNames[0] == "frame_0");
        REQUIRE(anim->frameNames[1] == "frame_1");
        REQUIRE(anim->frameNames[2] == "frame_2");
    }
    
    SECTION("Non-looping animation") {
        const AnimationData* anim = atlas.getAnimation("no_loop");
        REQUIRE(anim->loop == false);
    }
}

TEST_CASE("TextureAtlas lists animation names", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.loadFromFile("test_texture.png", "test_metadata.json");
    
    auto names = atlas.getAnimationNames();
    REQUIRE(names.size() == 2);
    
    // Should contain both animations (order not guaranteed)
    bool hasTestAnim = false;
    bool hasNoLoop = false;
    for (const auto& name : names) {
        if (name == "test_anim") hasTestAnim = true;
        if (name == "no_loop") hasNoLoop = true;
    }
    
    REQUIRE(hasTestAnim);
    REQUIRE(hasNoLoop);
}

TEST_CASE("TextureAtlas checks animation existence", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.loadFromFile("test_texture.png", "test_metadata.json");
    
    REQUIRE(atlas.hasAnimation("test_anim"));
    REQUIRE_FALSE(atlas.hasAnimation("does_not_exist"));
}

TEST_CASE("TextureAtlas handles manual frame addition", "[textureatlas][rendering]") {
    TextureAtlas atlas;
    
    SpriteFrame frame("manual_frame", sf::IntRect(0, 0, 32, 32), {16, 16});
    atlas.addFrame(frame);
    
    const SpriteFrame* retrieved = atlas.getFrame("manual_frame");
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->name == "manual_frame");
    REQUIRE(retrieved->rect.width == 32);
    REQUIRE(retrieved->origin.x == 16.0f);
}

TEST_CASE("TextureAtlas handles manual animation addition", "[textureatlas][rendering]") {
    TextureAtlas atlas;
    
    // Add frames first
    atlas.addFrame(SpriteFrame("f0", sf::IntRect(0, 0, 16, 16)));
    atlas.addFrame(SpriteFrame("f1", sf::IntRect(16, 0, 16, 16)));
    
    // Add animation
    AnimationData anim("manual_anim", {"f0", "f1"}, 0.15f, true);
    atlas.addAnimation(anim);
    
    const AnimationData* retrieved = atlas.getAnimation("manual_anim");
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->getFrameCount() == 2);
    REQUIRE(retrieved->frameDuration == 0.15f);
}

TEST_CASE("TextureAtlas validates animation frame references", "[textureatlas][rendering]") {
    TextureAtlas atlas;
    
    // Add only one frame
    atlas.addFrame(SpriteFrame("f0", sf::IntRect(0, 0, 16, 16)));
    
    // Try to add animation with non-existent frame
    AnimationData anim("bad_anim", {"f0", "f1"}, 0.1f, true);
    atlas.addAnimation(anim);
    
    // Should not add the animation
    REQUIRE_FALSE(atlas.hasAnimation("bad_anim"));
}

TEST_CASE("AnimationData per-frame durations", "[textureatlas][rendering]") {
    AnimationData anim;
    anim.frameNames = {"f0", "f1", "f2"};
    anim.frameDuration = 0.1f;
    anim.frameDurations = {0.05f, 0.15f, 0.2f};
    
    SECTION("Uses per-frame duration when available") {
        REQUIRE(anim.getDuration(0) == 0.05f);
        REQUIRE(anim.getDuration(1) == 0.15f);
        REQUIRE(anim.getDuration(2) == 0.2f);
    }
    
    SECTION("Falls back to default duration") {
        REQUIRE(anim.getDuration(3) == 0.1f);  // Out of bounds
    }
}

TEST_CASE("TextureAtlas default origin point", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    
    // Create metadata without explicit origin
    std::ofstream file("test_no_origin.json");
    file << R"({
        "texture": "test_texture.png",
        "frames": [
            {"name": "frame_0", "x": 0, "y": 0, "w": 32, "h": 48}
        ],
        "animations": []
    })";
    file.close();
    
    TextureAtlas atlas;
    atlas.loadFromFile("test_texture.png", "test_no_origin.json");
    
    const SpriteFrame* frame = atlas.getFrame("frame_0");
    REQUIRE(frame != nullptr);
    
    // Default origin should be bottom-center
    REQUIRE(frame->origin.x == 16.0f);  // width / 2
    REQUIRE(frame->origin.y == 48.0f);  // height
    
    std::remove("test_no_origin.json");
}

TEST_CASE("TextureAtlas clears data", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.loadFromFile("test_texture.png", "test_metadata.json");
    
    REQUIRE(atlas.getFrameCount() > 0);
    REQUIRE(atlas.getAnimationCount() > 0);
    
    atlas.clear();
    
    REQUIRE(atlas.getFrameCount() == 0);
    REQUIRE(atlas.getAnimationCount() == 0);
}