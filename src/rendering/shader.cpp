#include "shader.h"
#include "platform/file_system.h"
#include "platform/platform.h"
#include "platform/logging.h"
#include <bgfx/bgfx.h>

namespace Engine {

Shader::~Shader() {
    destroy();
}

bool Shader::load(const std::string& vertexBaseName, const std::string& fragmentBaseName) {
    bgfx::ShaderHandle vsh = loadShader(vertexBaseName);
    bgfx::ShaderHandle fsh = loadShader(fragmentBaseName);

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh)) {
        Log::error("Failed to load shaders: {} / {}", vertexBaseName, fragmentBaseName);
        return false;
    }

    program = bgfx::createProgram(vsh, fsh, true);
    if (!bgfx::isValid(program)) {
        Log::error("Failed to create shader program");
        return false;
    }

    Log::info("Shader program created: {} / {}", vertexBaseName, fragmentBaseName);
    return true;
}

bgfx::ShaderHandle Shader::loadShader(const std::string& baseName) {
    std::string path = selectShaderPath(baseName);
    auto dataOpt = FileSystem::loadBinaryFile(path);
    if (!dataOpt) {
        Log::error("Failed to load shader binary: {}", path);
        return BGFX_INVALID_HANDLE;
    }
    const bgfx::Memory* mem = bgfx::copy(dataOpt->data(), static_cast<uint32_t>(dataOpt->size()));
    return bgfx::createShader(mem);
}

std::string Shader::selectShaderPath(const std::string& baseName) {
    bgfx::RendererType::Enum backend = bgfx::getRendererType();
    std::string ext;
    switch (backend) {
        case bgfx::RendererType::OpenGL:
        case bgfx::RendererType::OpenGLES: ext = ".gl.bin"; break;
        case bgfx::RendererType::Vulkan: ext = ".vk.bin"; break;
        case bgfx::RendererType::Metal: ext = ".mtl.bin"; break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12: ext = ".dx11.bin"; break;
        default: ext = ".gl.bin"; break;
    }
    return Platform::getResourcePath("shaders/" + baseName + ext);
}

void Shader::destroy() {
    if (bgfx::isValid(program)) {
        bgfx::destroy(program);
        program = BGFX_INVALID_HANDLE;
    }
}

} // namespace Engine

