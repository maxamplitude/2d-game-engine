#pragma once
#include "math/vector.h"
#include "core/types.h"
#include "shader.h"
#include "texture.h"
#include <bgfx/bgfx.h>
#include <vector>

namespace Engine {

struct SpriteBatchVertex {
    Vec3 position;
    Vec2 texCoord;
    uint32_t color;

    static void init() {
        layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }

    static bgfx::VertexLayout layout;
};

struct SpriteDrawData {
    bgfx::TextureHandle texture;
    Vec2 position;
    Vec2 size;
    Vec4 uvRect;  // x, y, w, h (normalized 0-1)
    Vec2 origin;
    float rotation;
    Color color;
};

class SpriteBatch {
public:
    SpriteBatch();
    ~SpriteBatch();
    
    void begin(const Mat4& viewProj);
    void draw(const SpriteDrawData& sprite);
    void end();
    
private:
    Shader spriteShader;
    bgfx::UniformHandle u_mvp;
    bgfx::UniformHandle s_texture;

    std::vector<SpriteBatchVertex> vertices;
    std::vector<uint16_t> indices;

    Mat4 viewProjMatrix;
    uint32_t spriteCount;
    bgfx::TextureHandle currentTexture;
    bool initialized;

    void flush();
};

} // namespace Engine