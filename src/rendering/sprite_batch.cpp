#include "sprite_batch.h"
#include "platform/logging.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <cstring>

namespace Engine {

bgfx::VertexLayout SpriteBatchVertex::layout;

namespace {
constexpr uint32_t kMaxSpritesPerBatch = 1024;
} // namespace

SpriteBatch::SpriteBatch()
    : u_mvp(BGFX_INVALID_HANDLE),
      s_texture(BGFX_INVALID_HANDLE),
      spriteCount(0),
      currentTexture(BGFX_INVALID_HANDLE),
      initialized(false) {
    SpriteBatchVertex::init();

    if (!spriteShader.load("sprite.vert", "sprite.frag")) {
        Log::critical("SpriteBatch failed to load sprite shader");
        return;
    }

    u_mvp = bgfx::createUniform("u_mvp", bgfx::UniformType::Mat4);
    s_texture = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);

    if (!bgfx::isValid(u_mvp) || !bgfx::isValid(s_texture)) {
        Log::critical("SpriteBatch failed to create uniforms");
        return;
    }

    vertices.reserve(kMaxSpritesPerBatch * 4);
    indices.reserve(kMaxSpritesPerBatch * 6);
    initialized = true;
}

SpriteBatch::~SpriteBatch() {
    if (bgfx::isValid(u_mvp)) {
        bgfx::destroy(u_mvp);
    }
    if (bgfx::isValid(s_texture)) {
        bgfx::destroy(s_texture);
    }
    spriteShader.destroy();
}

void SpriteBatch::begin(const Mat4& viewProj) {
    if (!initialized) return;
    viewProjMatrix = viewProj;
    vertices.clear();
    indices.clear();
    spriteCount = 0;
    currentTexture = BGFX_INVALID_HANDLE;
}

void SpriteBatch::draw(const SpriteDrawData& sprite) {
    if (!initialized) return;
    if (!bgfx::isValid(sprite.texture)) return;

    // Flush when texture changes to ensure correct binding.
    if (bgfx::isValid(currentTexture) && currentTexture.idx != sprite.texture.idx) {
        flush();
    }
    currentTexture = sprite.texture;

    const float rad = toRadians(sprite.rotation);
    const float c = std::cos(rad);
    const float s = std::sin(rad);

    // Local quad corners
    Vec2 local[4] = {
        Vec2(0.0f, 0.0f),
        Vec2(sprite.size.x, 0.0f),
        Vec2(sprite.size.x, sprite.size.y),
        Vec2(0.0f, sprite.size.y)
    };

    const uint32_t packedColor = sprite.color.toUint32();

    auto rotateTranslate = [&](const Vec2& p) -> Vec2 {
        Vec2 centered = p - sprite.origin;
        return Vec2(centered.x * c - centered.y * s,
                    centered.x * s + centered.y * c) + sprite.position;
    };

    const uint16_t base = static_cast<uint16_t>(vertices.size());
    vertices.push_back({Vec3(rotateTranslate(local[0]), 0.0f), Vec2(sprite.uvRect.x, sprite.uvRect.y), packedColor});
    vertices.push_back({Vec3(rotateTranslate(local[1]), 0.0f), Vec2(sprite.uvRect.x + sprite.uvRect.z, sprite.uvRect.y), packedColor});
    vertices.push_back({Vec3(rotateTranslate(local[2]), 0.0f), Vec2(sprite.uvRect.x + sprite.uvRect.z, sprite.uvRect.y + sprite.uvRect.w), packedColor});
    vertices.push_back({Vec3(rotateTranslate(local[3]), 0.0f), Vec2(sprite.uvRect.x, sprite.uvRect.y + sprite.uvRect.w), packedColor});

    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);

    spriteCount++;
    if (spriteCount >= kMaxSpritesPerBatch) {
        flush();
    }
}

void SpriteBatch::end() {
    if (!initialized) return;
    flush();
}

void SpriteBatch::flush() {
    if (!initialized) return;
    if (vertices.empty() || !bgfx::isValid(currentTexture)) {
        vertices.clear();
        indices.clear();
        spriteCount = 0;
        return;
    }

    const uint32_t vcount = static_cast<uint32_t>(vertices.size());
    const uint32_t icount = static_cast<uint32_t>(indices.size());

    if (bgfx::getAvailTransientVertexBuffer(vcount, SpriteBatchVertex::layout) < vcount ||
        bgfx::getAvailTransientIndexBuffer(icount) < icount) {
        Log::warn("SpriteBatch skipped draw: insufficient transient buffers (v={}, i={})", vcount, icount);
        vertices.clear();
        indices.clear();
        spriteCount = 0;
        return;
    }

    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer tib;
    bgfx::allocTransientVertexBuffer(&tvb, vcount, SpriteBatchVertex::layout);
    bgfx::allocTransientIndexBuffer(&tib, icount);

    std::memcpy(tvb.data, vertices.data(), vcount * sizeof(SpriteBatchVertex));
    std::memcpy(tib.data, indices.data(), icount * sizeof(uint16_t));

    bgfx::setUniform(u_mvp, glm::value_ptr(viewProjMatrix));
    bgfx::setVertexBuffer(0, &tvb);
    bgfx::setIndexBuffer(&tib);
    bgfx::setTexture(0, s_texture, currentTexture);
    bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_MSAA);
    bgfx::submit(0, spriteShader.getProgram());

    vertices.clear();
    indices.clear();
    spriteCount = 0;
    currentTexture = BGFX_INVALID_HANDLE;
}

} // namespace Engine

