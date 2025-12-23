// src/rendering/render_queue.h
#pragma once
#include <vector>
#include "core/transform.h"
#include "math/rectangle.h"
#include "rendering/sprite_batch.h"

namespace Engine {

// Render item submitted to the queue
struct RenderItem {
    float depth = 0.0f;                 // Z-order: higher = further back
    SpriteDrawData sprite{};            // Sprite draw data (texture, size, UVs, color)
    Transform transform;                // Transform in world space
    
    RenderItem() = default;
    RenderItem(float d, const SpriteDrawData& spr, const Transform& t)
        : depth(d), sprite(spr), transform(t) {}
};

// Manages depth-sorted rendering of sprites
class RenderQueue {
public:
    RenderQueue() = default;
    
    // Submission
    void submit(const RenderItem& item);
    void submit(float depth, const SpriteDrawData& sprite, const Transform& transform);
    
    // Lifecycle
    void clear();
    void sort();  // Must call before render()

    // Rendering (batch type must expose begin(viewProj), draw(sprite), end())
    template <typename BatchT>
    void render(BatchT& batch, const Mat4& viewProj);
    
    // Camera integration (optional)
    void setCameraTransform(const Transform& camera);
    void setCullingBounds(const Rectangle& bounds);
    void enableCulling(bool enabled) { cullingEnabled = enabled; }
    
    // Queries
    size_t size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    
    // Stats (for debugging/profiling)
    size_t getCulledCount() const { return culledCount; }
    void resetStats() { culledCount = 0; }
    
private:
    std::vector<RenderItem> items;
    Transform cameraTransform;
    Rectangle cullingBounds;
    bool cullingEnabled = false;
    size_t culledCount = 0;
    
    bool shouldCull(const RenderItem& item) const;
    Vec2 worldToScreen(const Vec2& worldPos) const;
    SpriteDrawData buildDrawData(const RenderItem& item) const;
};

template <typename BatchT>
void RenderQueue::render(BatchT& batch, const Mat4& viewProj) {
    culledCount = 0;
    batch.begin(viewProj);
    
    for (const auto& item : items) {
        if (cullingEnabled && shouldCull(item)) {
            ++culledCount;
            continue;
        }
        
        SpriteDrawData drawData = buildDrawData(item);
        batch.draw(drawData);
    }
    
    batch.end();
}

} // namespace Engine