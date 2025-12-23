#include "quad_renderer.h"
#include <bgfx/bgfx.h>
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
        return false;
    }
    u_viewProj = bgfx::createUniform("u_viewProj", bgfx::UniformType::Mat4);
    s_texture = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    return bgfx::isValid(u_viewProj) && bgfx::isValid(s_texture);
}

void QuadRenderer::shutdown() {
    if (bgfx::isValid(u_viewProj)) bgfx::destroy(u_viewProj);
    if (bgfx::isValid(s_texture)) bgfx::destroy(s_texture);
    shader.destroy();
}

void QuadRenderer::draw(const Mat4& viewProj, const Mat4& model, bgfx::TextureHandle texture, const Color& color, uint16_t viewId) {
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
    bgfx::setUniform(u_viewProj, glm::value_ptr(viewProj));
    bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_MSAA);
    bgfx::submit(viewId, shader.getProgram());
}

} // namespace Engine

