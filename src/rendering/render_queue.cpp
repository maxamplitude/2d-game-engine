// src/rendering/render_queue.cpp
#include "render_queue.h"
#include <algorithm>

namespace Engine {

void RenderQueue::submit(const RenderItem& item) {
    items.push_back(item);
}

void RenderQueue::submit(float depth, const SpriteDrawData& sprite, const Transform& transform) {
    RenderItem item;
    item.depth = depth;
    item.sprite = sprite;
    item.transform = transform;
    items.push_back(item);
}

void RenderQueue::clear() {
    items.clear();
    culledCount = 0;
}

void RenderQueue::sort() {
    // Painter's algorithm: higher depth (further away) draws first
    std::sort(items.begin(), items.end(),
        [](const RenderItem& a, const RenderItem& b) {
            return a.depth > b.depth;
        });
}

void RenderQueue::setCameraTransform(const Transform& camera) {
    cameraTransform = camera;
}

void RenderQueue::setCullingBounds(const Rectangle& bounds) {
    cullingBounds = bounds;
}

bool RenderQueue::shouldCull(const RenderItem& item) const {
    // Simple point-in-rectangle culling using world position
    return !cullingBounds.contains(item.transform.position);
}

Vec2 RenderQueue::worldToScreen(const Vec2& worldPos) const {
    // Basic camera offset subtraction
    return worldPos - cameraTransform.position;
}

SpriteDrawData RenderQueue::buildDrawData(const RenderItem& item) const {
    SpriteDrawData result = item.sprite;
    result.position = worldToScreen(item.transform.position);
    result.rotation = item.transform.rotation;
    result.size = item.sprite.size * item.transform.scale;
    return result;
}

} // namespace Engine