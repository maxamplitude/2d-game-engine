#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "rendering/texture_atlas.h"
#include <fstream>
#include <cstdio>

using namespace Engine;

// Test fixture - creates temporary test files
class TextureAtlasFixture {
public:
    TextureAtlasFixture() {
        create_test_texture();
        create_test_metadata();
    }
    
    ~TextureAtlasFixture() {
        // Cleanup
        std::remove("test_texture.png");
        std::remove("test_metadata.json");
    }
    
private:
    void create_test_texture() {
        std::ofstream file("test_texture.png", std::ios::binary);
        file << "not a real png";
    }
    
    void create_test_metadata() {
        std::ofstream file("test_metadata.json");
        file << R"({
            "texture": "test_texture.png",
            "texture_width": 64,
            "texture_height": 32,
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
                    "frame_duration": 0.1,
                    "loop": true
                },
                {
                    "name": "no_loop",
                    "frames": ["frame_0", "frame_1"],
                    "frame_duration": 0.2,
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
    
    bool loaded = atlas.load_from_file("test_texture.png", "test_metadata.json");
    
    REQUIRE(loaded);
    REQUIRE(atlas.get_frame_count() == 4);
    REQUIRE(atlas.get_animation_count() == 2);
    REQUIRE(atlas.get_texture_width() == 64);
    REQUIRE(atlas.get_texture_height() == 32);
}

TEST_CASE("TextureAtlas retrieves frames", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.load_from_file("test_texture.png", "test_metadata.json");
    
    SECTION("Existing frame") {
        const SpriteFrame* frame = atlas.get_frame("frame_0");
        REQUIRE(frame != nullptr);
        REQUIRE(frame->name == "frame_0");
        REQUIRE(frame->pixel_rect.x == 0);
        REQUIRE(frame->pixel_rect.y == 0);
        REQUIRE(frame->pixel_rect.z == 16);
        REQUIRE(frame->pixel_rect.w == 16);
        REQUIRE(frame->size.x == 16.0f);
        REQUIRE(frame->size.y == 16.0f);
        REQUIRE(frame->uv_rect.x == Catch::Approx(0.0f));
        REQUIRE(frame->uv_rect.z == Catch::Approx(0.25f)); // 16 / 64
    }
    
    SECTION("Non-existent frame") {
        const SpriteFrame* frame = atlas.get_frame("does_not_exist");
        REQUIRE(frame == nullptr);
    }
}

TEST_CASE("TextureAtlas retrieves animations", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.load_from_file("test_texture.png", "test_metadata.json");
    
    SECTION("Existing animation") {
        const AnimationData* anim = atlas.get_animation("test_anim");
        REQUIRE(anim != nullptr);
        REQUIRE(anim->name == "test_anim");
        REQUIRE(anim->get_frame_count() == 3);
        REQUIRE(anim->frame_duration == 0.1f);
        REQUIRE(anim->loop == true);
    }
    
    SECTION("Animation frame names") {
        const AnimationData* anim = atlas.get_animation("test_anim");
        REQUIRE(anim->frame_names[0] == "frame_0");
        REQUIRE(anim->frame_names[1] == "frame_1");
        REQUIRE(anim->frame_names[2] == "frame_2");
    }
    
    SECTION("Non-looping animation") {
        const AnimationData* anim = atlas.get_animation("no_loop");
        REQUIRE(anim->loop == false);
    }
}

TEST_CASE("TextureAtlas lists animation names", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.load_from_file("test_texture.png", "test_metadata.json");
    
    auto names = atlas.get_animation_names();
    REQUIRE(names.size() == 2);
    
    // Should contain both animations (order not guaranteed)
    bool has_test_anim = false;
    bool has_no_loop = false;
    for (const auto& name : names) {
        if (name == "test_anim") has_test_anim = true;
        if (name == "no_loop") has_no_loop = true;
    }
    
    REQUIRE(has_test_anim);
    REQUIRE(has_no_loop);
}

TEST_CASE("TextureAtlas checks animation existence", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.load_from_file("test_texture.png", "test_metadata.json");
    
    REQUIRE(atlas.has_animation("test_anim"));
    REQUIRE_FALSE(atlas.has_animation("does_not_exist"));
}

TEST_CASE("TextureAtlas handles manual frame addition", "[textureatlas][rendering]") {
    TextureAtlas atlas;
    
    SpriteFrame frame("manual_frame", glm::ivec4(0, 0, 32, 32), glm::vec2(16, 16));
    atlas.add_frame(frame);
    
    const SpriteFrame* retrieved = atlas.get_frame("manual_frame");
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->name == "manual_frame");
    REQUIRE(retrieved->pixel_rect.z == 32);
    REQUIRE(retrieved->origin.x == 16.0f);
}

TEST_CASE("TextureAtlas handles manual animation addition", "[textureatlas][rendering]") {
    TextureAtlas atlas;
    
    // Add frames first
    SpriteFrame f0("f0", glm::ivec4(0, 0, 16, 16));
    SpriteFrame f1("f1", glm::ivec4(16, 0, 16, 16));
    atlas.add_frame(f0);
    atlas.add_frame(f1);
    
    // Add animation
    AnimationData anim("manual_anim", {"f0", "f1"}, 0.15f, true);
    atlas.add_animation(anim);
    
    const AnimationData* retrieved = atlas.get_animation("manual_anim");
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->get_frame_count() == 2);
    REQUIRE(retrieved->frame_duration == 0.15f);
}

TEST_CASE("TextureAtlas validates animation frame references", "[textureatlas][rendering]") {
    TextureAtlas atlas;
    
    // Add only one frame
    SpriteFrame f0("f0", glm::ivec4(0, 0, 16, 16));
    atlas.add_frame(f0);
    
    // Try to add animation with non-existent frame
    AnimationData anim("bad_anim", {"f0", "f1"}, 0.1f, true);
    atlas.add_animation(anim);
    
    // Should not add the animation
    REQUIRE_FALSE(atlas.has_animation("bad_anim"));
}

TEST_CASE("AnimationData per-frame durations", "[textureatlas][rendering]") {
    AnimationData anim;
    anim.frame_names = {"f0", "f1", "f2"};
    anim.frame_duration = 0.1f;
    anim.frame_durations = {0.05f, 0.15f, 0.2f};
    
    SECTION("Uses per-frame duration when available") {
        REQUIRE(anim.get_duration(0) == 0.05f);
        REQUIRE(anim.get_duration(1) == 0.15f);
        REQUIRE(anim.get_duration(2) == 0.2f);
    }
    
    SECTION("Falls back to default duration") {
        REQUIRE(anim.get_duration(3) == 0.1f);  // Out of bounds
    }
}

TEST_CASE("TextureAtlas default origin point", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    
    // Create metadata without explicit origin
    std::ofstream file("test_no_origin.json");
    file << R"({
        "texture": "test_texture.png",
        "texture_width": 32,
        "texture_height": 48,
        "frames": [
            {"name": "frame_0", "x": 0, "y": 0, "w": 32, "h": 48}
        ],
        "animations": []
    })";
    file.close();
    
    TextureAtlas atlas;
    atlas.load_from_file("test_texture.png", "test_no_origin.json");
    
    const SpriteFrame* frame = atlas.get_frame("frame_0");
    REQUIRE(frame != nullptr);
    
    // Default origin should be bottom-center
    REQUIRE(frame->origin.x == 16.0f);  // width / 2
    REQUIRE(frame->origin.y == 48.0f);  // height
    
    std::remove("test_no_origin.json");
}

TEST_CASE("TextureAtlas clears data", "[textureatlas][rendering]") {
    TextureAtlasFixture fixture;
    TextureAtlas atlas;
    atlas.load_from_file("test_texture.png", "test_metadata.json");
    
    REQUIRE(atlas.get_frame_count() > 0);
    REQUIRE(atlas.get_animation_count() > 0);
    
    atlas.clear();
    
    REQUIRE(atlas.get_frame_count() == 0);
    REQUIRE(atlas.get_animation_count() == 0);
}