#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "rendering/render_queue.h"

using namespace Engine;
using Catch::Approx;

namespace {

struct RecordingBatch {
    std::vector<SpriteDrawData> drawn;
    Mat4 lastViewProj{};

    void begin(const Mat4& viewProj) {
        lastViewProj = viewProj;
        drawn.clear();
    }

    void draw(const SpriteDrawData& sprite) {
        drawn.push_back(sprite);
    }

    void end() {}
};

SpriteDrawData createTestSprite(const Vec2& size = Vec2(16.0f, 16.0f), const Color& color = Color::White) {
    SpriteDrawData data{};
    data.texture = BGFX_INVALID_HANDLE; // We do not need a valid GPU handle for unit tests
    data.position = {0.0f, 0.0f};
    data.size = size;
    data.uvRect = Vec4(0.0f, 0.0f, 1.0f, 1.0f);
    data.origin = Vec2(0.0f, 0.0f);
    data.rotation = 0.0f;
    data.color = color;
    return data;
}

const Mat4 kIdentityViewProj = Mat4(1.0f);

} // namespace

TEST_CASE("RenderQueue default construction", "[renderqueue][rendering]") {
    RenderQueue queue;
    
    REQUIRE(queue.empty());
    REQUIRE(queue.size() == 0);
}

TEST_CASE("RenderQueue submission", "[renderqueue][rendering]") {
    RenderQueue queue;
    
    Transform transform;
    transform.position = {100.0f, 200.0f};
    
    SpriteDrawData sprite = createTestSprite();
    
    SECTION("Submit via full item") {
        RenderItem item(10.0f, sprite, transform);
        queue.submit(item);
        
        REQUIRE(queue.size() == 1);
    }
    
    SECTION("Submit via convenience method") {
        queue.submit(10.0f, sprite, transform);
        
        REQUIRE(queue.size() == 1);
    }
    
    SECTION("Multiple submissions") {
        queue.submit(10.0f, sprite, transform);
        queue.submit(20.0f, sprite, transform);
        queue.submit(30.0f, sprite, transform);
        
        REQUIRE(queue.size() == 3);
    }
}

TEST_CASE("RenderQueue clears items", "[renderqueue][rendering]") {
    RenderQueue queue;
    Transform transform;
    SpriteDrawData sprite = createTestSprite();
    
    queue.submit(10.0f, sprite, transform);
    queue.submit(20.0f, sprite, transform);
    
    REQUIRE(queue.size() == 2);
    
    queue.clear();
    
    REQUIRE(queue.empty());
    REQUIRE(queue.size() == 0);
}

TEST_CASE("RenderQueue sorts by depth", "[renderqueue][rendering]") {
    RenderQueue queue;
    Transform transform;
    
    SpriteDrawData s1 = createTestSprite();
    SpriteDrawData s2 = createTestSprite();
    SpriteDrawData s3 = createTestSprite();
    SpriteDrawData s4 = createTestSprite();

    // Encode depth markers in uvRect.x so we can verify order after render
    s1.uvRect.x = 30.0f;
    s2.uvRect.x = 10.0f;
    s3.uvRect.x = 50.0f;
    s4.uvRect.x = 20.0f;
    
    queue.submit(30.0f, s1, transform);
    queue.submit(10.0f, s2, transform);
    queue.submit(50.0f, s3, transform);
    queue.submit(20.0f, s4, transform);
    
    queue.sort();
    
    RecordingBatch batch;
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(batch.drawn.size() == 4);
    REQUIRE(batch.drawn[0].uvRect.x == Approx(50.0f));
    REQUIRE(batch.drawn[1].uvRect.x == Approx(30.0f));
    REQUIRE(batch.drawn[2].uvRect.x == Approx(20.0f));
    REQUIRE(batch.drawn[3].uvRect.x == Approx(10.0f));
}

TEST_CASE("RenderQueue renders via batch", "[renderqueue][rendering]") {
    RenderQueue queue;
    
    Transform transform;
    transform.position = {100.0f, 100.0f};
    
    SpriteDrawData sprite = createTestSprite();
    
    queue.submit(10.0f, sprite, transform);
    queue.sort();
    
    RecordingBatch batch;
    
    REQUIRE_NOTHROW(queue.render(batch, kIdentityViewProj));
    REQUIRE(batch.drawn.size() == 1);
}

TEST_CASE("RenderQueue rendering with depth order", "[renderqueue][rendering]") {
    RenderQueue queue;
    
    Transform t1, t2, t3;
    t1.position = {100.0f, 100.0f};
    t2.position = {100.0f, 100.0f};
    t3.position = {100.0f, 100.0f};
    
    SpriteDrawData s1 = createTestSprite(); s1.uvRect.x = 10.0f;
    SpriteDrawData s2 = createTestSprite(); s2.uvRect.x = 50.0f;
    SpriteDrawData s3 = createTestSprite(); s3.uvRect.x = 30.0f;
    
    queue.submit(10.0f, s1, t1);   // Foreground
    queue.submit(50.0f, s2, t2);   // Background
    queue.submit(30.0f, s3, t3);   // Middle
    
    queue.sort();
    
    RecordingBatch batch;
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(queue.size() == 3);
    REQUIRE(batch.drawn.size() == 3);
    REQUIRE(batch.drawn[0].uvRect.x == Approx(50.0f));
    REQUIRE(batch.drawn[1].uvRect.x == Approx(30.0f));
    REQUIRE(batch.drawn[2].uvRect.x == Approx(10.0f));
}

TEST_CASE("RenderQueue camera transform", "[renderqueue][rendering]") {
    RenderQueue queue;
    
    Transform cameraTransform;
    cameraTransform.position = {100.0f, 50.0f};
    
    queue.setCameraTransform(cameraTransform);
    
    Transform entityTransform;
    entityTransform.position = {200.0f, 150.0f};
    
    SpriteDrawData sprite = createTestSprite();
    queue.submit(10.0f, sprite, entityTransform);
    queue.sort();
    
    RecordingBatch batch;
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(batch.drawn.size() == 1);
    REQUIRE(batch.drawn[0].position.x == Approx(100.0f));
    REQUIRE(batch.drawn[0].position.y == Approx(100.0f));
}

TEST_CASE("RenderQueue frustum culling", "[renderqueue][rendering]") {
    RenderQueue queue;
    queue.enableCulling(true);
    
    Rectangle cullBounds(0.0f, 0.0f, 800.0f, 600.0f);
    queue.setCullingBounds(cullBounds);
    
    SpriteDrawData sprite = createTestSprite();
    
    SECTION("Entity inside bounds is not culled") {
        Transform transform;
        transform.position = {400.0f, 300.0f};  // Center of bounds
        
        queue.submit(10.0f, sprite, transform);
        queue.sort();
        
        RecordingBatch batch;
        queue.render(batch, kIdentityViewProj);
        
        REQUIRE(queue.getCulledCount() == 0);
        REQUIRE(batch.drawn.size() == 1);
    }
    
    SECTION("Entity outside bounds is culled") {
        Transform transform;
        transform.position = {1000.0f, 1000.0f};  // Outside bounds
        
        queue.submit(10.0f, sprite, transform);
        queue.sort();
        
        RecordingBatch batch;
        queue.render(batch, kIdentityViewProj);
        
        REQUIRE(queue.getCulledCount() == 1);
        REQUIRE(batch.drawn.empty());
    }
    
    SECTION("Mixed culling") {
        Transform t1, t2, t3;
        t1.position = {400.0f, 300.0f};   // Inside
        t2.position = {-100.0f, -100.0f}; // Outside
        t3.position = {700.0f, 500.0f};   // Inside
        
        queue.submit(10.0f, sprite, t1);
        queue.submit(20.0f, sprite, t2);
        queue.submit(30.0f, sprite, t3);
        queue.sort();
        
        RecordingBatch batch;
        queue.render(batch, kIdentityViewProj);
        
        REQUIRE(queue.getCulledCount() == 1);
        REQUIRE(batch.drawn.size() == 2);
    }
}

TEST_CASE("RenderQueue culling can be disabled", "[renderqueue][rendering]") {
    RenderQueue queue;
    queue.enableCulling(false);  // Explicitly disabled
    
    Rectangle cullBounds(0.0f, 0.0f, 800.0f, 600.0f);
    queue.setCullingBounds(cullBounds);
    
    Transform transform;
    transform.position = {1000.0f, 1000.0f};  // Outside bounds
    
    SpriteDrawData sprite = createTestSprite();
    queue.submit(10.0f, sprite, transform);
    queue.sort();
    
    RecordingBatch batch;
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(queue.getCulledCount() == 0);
    REQUIRE(batch.drawn.size() == 1);
}

TEST_CASE("RenderQueue stats reset", "[renderqueue][rendering]") {
    RenderQueue queue;
    queue.enableCulling(true);
    
    Rectangle cullBounds(0.0f, 0.0f, 800.0f, 600.0f);
    queue.setCullingBounds(cullBounds);
    
    Transform transform;
    transform.position = {1000.0f, 1000.0f};  // Outside
    
    SpriteDrawData sprite = createTestSprite();
    queue.submit(10.0f, sprite, transform);
    queue.sort();
    
    RecordingBatch batch;
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(queue.getCulledCount() == 1);
    
    queue.resetStats();
    REQUIRE(queue.getCulledCount() == 0);
}

TEST_CASE("RenderQueue applies transform rotation and scale", "[renderqueue][rendering]") {
    RenderQueue queue;
    
    Transform transform;
    transform.position = {100.0f, 100.0f};
    transform.rotation = 45.0f;
    transform.scale = {2.0f, 2.0f};
    
    SpriteDrawData sprite = createTestSprite(Vec2(16.0f, 16.0f));
    queue.submit(10.0f, sprite, transform);
    queue.sort();
    
    RecordingBatch batch;
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(batch.drawn.size() == 1);
    REQUIRE(batch.drawn[0].rotation == Approx(45.0f));
    REQUIRE(batch.drawn[0].size.x == Approx(32.0f));
    REQUIRE(batch.drawn[0].size.y == Approx(32.0f));
}

TEST_CASE("RenderQueue handles empty render", "[renderqueue][rendering]") {
    RenderQueue queue;
    RecordingBatch batch;
    
    REQUIRE_NOTHROW(queue.render(batch, kIdentityViewProj));
    REQUIRE(batch.drawn.empty());
}

TEST_CASE("RenderQueue depth stability", "[renderqueue][rendering]") {
    RenderQueue queue;
    Transform transform;
    SpriteDrawData sprite = createTestSprite();
    
    queue.submit(10.0f, sprite, transform);
    queue.submit(10.0f, sprite, transform);
    queue.submit(10.0f, sprite, transform);
    
    queue.sort();
    
    RecordingBatch batch;
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(queue.size() == 3);
    REQUIRE(batch.drawn.size() == 3);
}

TEST_CASE("RenderQueue reusability", "[renderqueue][rendering]") {
    RenderQueue queue;
    Transform transform;
    SpriteDrawData sprite = createTestSprite();
    
    RecordingBatch batch;
    
    // First frame
    queue.submit(10.0f, sprite, transform);
    queue.sort();
    queue.render(batch, kIdentityViewProj);
    REQUIRE(batch.drawn.size() == 1);
    
    // Clear and reuse
    queue.clear();
    REQUIRE(queue.empty());
    
    // Second frame
    queue.submit(20.0f, sprite, transform);
    queue.submit(30.0f, sprite, transform);
    queue.sort();
    queue.render(batch, kIdentityViewProj);
    
    REQUIRE(queue.size() == 2);
    REQUIRE(batch.drawn.size() == 2);
}