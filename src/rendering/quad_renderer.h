#pragma once
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "math/vector.h"
#include "core/types.h"
#include <bgfx/bgfx.h>

namespace Engine {

struct SpriteVertex {
    float x, y, z;
    float u, v;
    uint32_t color;

    static bgfx::VertexLayout layout;
    static void init();
};

class QuadRenderer {
public:
    QuadRenderer() = default;
    ~QuadRenderer() = default;

    bool init();
    void shutdown();

    void draw(const Mat4& viewProj, const Mat4& model, bgfx::TextureHandle texture, const Color& color, uint16_t viewId = 0);

private:
    Shader shader;
    bgfx::UniformHandle u_viewProj = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle s_texture = BGFX_INVALID_HANDLE;
};

} // namespace Engine

