#pragma once
#include <bgfx/bgfx.h>
#include <string>

namespace Engine {

class Texture {
public:
    Texture() = default;
    ~Texture();

    bool loadFromFile(const std::string& path);
    bool loadFromMemory(const void* data, uint32_t size);
    bool loadFromRGBA(uint16_t w, uint16_t h, const uint8_t* rgba, bool generateMips = false);

    uint16_t getWidth() const { return width; }
    uint16_t getHeight() const { return height; }
    bgfx::TextureHandle getHandle() const { return handle; }
    bool isValid() const { return bgfx::isValid(handle); }

    void destroy();

private:
    bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
    uint16_t width = 0;
    uint16_t height = 0;
};

} // namespace Engine

