#include "quad_renderer.h"
#include <bgfx/bgfx.h>
#include "platform/logging.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

namespace Engine {

bgfx::VertexLayout SpriteVertex::layout;

void SpriteVertex::init() {
    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
}

bool QuadRenderer::init() {
    SpriteVertex::init();
    if (!shader.load("sprite.vert", "sprite.frag")) {
        Log::critical("QuadRenderer: failed to load sprite shader");
        return false;
    }
    u_mvp = bgfx::createUniform("u_mvp", bgfx::UniformType::Mat4);
    s_texture = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    if (!bgfx::isValid(u_mvp) || !bgfx::isValid(s_texture)) {
        Log::critical("QuadRenderer: failed to create required uniforms");
        return false;
    }
    return true;
}

void QuadRenderer::shutdown() {
    if (bgfx::isValid(u_mvp)) bgfx::destroy(u_mvp);
    if (bgfx::isValid(s_texture)) bgfx::destroy(s_texture);
    shader.destroy();
}

void QuadRenderer::draw(const Mat4& viewProj, const Mat4& model, bgfx::TextureHandle texture, const Color& color, uint16_t viewId) {
    if (!bgfx::isValid(u_mvp) || !bgfx::isValid(s_texture) || !bgfx::isValid(shader.getProgram()) || !bgfx::isValid(texture)) {
        Log::warn("QuadRenderer::draw skipped due to invalid handles (uniforms/program/texture)");
        return;
    }
    SpriteVertex verts[4];
    const float w = 1.0f;
    const float h = 1.0f;
    uint32_t abgr = color.toUint32();
    verts[0] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, abgr};
    verts[1] = {w,    0.0f, 0.0f, 1.0f, 0.0f, abgr};
    verts[2] = {w,    h,    0.0f, 1.0f, 1.0f, abgr};
    verts[3] = {0.0f, h,    0.0f, 0.0f, 1.0f, abgr};

    const uint16_t indices[6] = {0, 1, 2, 0, 2, 3};

    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer tib;
    if (bgfx::getAvailTransientVertexBuffer(4, SpriteVertex::layout) < 4 ||
        bgfx::getAvailTransientIndexBuffer(6) < 6) {
        return;
    }
    bgfx::allocTransientVertexBuffer(&tvb, 4, SpriteVertex::layout);
    bgfx::allocTransientIndexBuffer(&tib, 6);

    std::memcpy(tvb.data, verts, sizeof(verts));
    std::memcpy(tib.data, indices, sizeof(indices));

    bgfx::setTransform(glm::value_ptr(model));
    bgfx::setVertexBuffer(0, &tvb, 0, 4);
    bgfx::setIndexBuffer(&tib, 0, 6);
    bgfx::setTexture(0, s_texture, texture);
    bgfx::setUniform(u_mvp, glm::value_ptr(viewProj));
    bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_MSAA);
    bgfx::submit(viewId, shader.getProgram());
}

} // namespace Engine

