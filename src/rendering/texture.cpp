#include "texture.h"
#include "platform/file_system.h"
#include "platform/logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine {

Texture::~Texture() {
    destroy();
}

bool Texture::loadFromFile(const std::string& path) {
    auto dataOpt = FileSystem::loadBinaryFile(path);
    if (!dataOpt) {
        Log::error("Failed to load texture file: {}", path);
        return false;
    }
    return loadFromMemory(dataOpt->data(), static_cast<uint32_t>(dataOpt->size()));
}

bool Texture::loadFromMemory(const void* data, uint32_t size) {
    int w, h, channels;
    stbi_uc* pixels = stbi_load_from_memory(
        static_cast<const stbi_uc*>(data), size,
        &w, &h, &channels, 4);

    if (!pixels) {
        Log::error("Failed to decode image: {}", stbi_failure_reason());
        return false;
    }

    width = static_cast<uint16_t>(w);
    height = static_cast<uint16_t>(h);

    const bgfx::Memory* mem = bgfx::copy(pixels, w * h * 4);
    handle = bgfx::createTexture2D(
        width, height,
        false,
        1,
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
        mem);

    stbi_image_free(pixels);

    if (!bgfx::isValid(handle)) {
        Log::error("Failed to create BGFX texture");
        return false;
    }

    Log::info("Texture loaded: {}x{}", width, height);
    return true;
}

void Texture::destroy() {
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
        handle = BGFX_INVALID_HANDLE;
    }
}

} // namespace Engine

