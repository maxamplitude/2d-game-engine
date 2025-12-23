#pragma once
#include <bgfx/bgfx.h>
#include <string>

namespace Engine {

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool load(const std::string& vertexBaseName, const std::string& fragmentBaseName);
    bgfx::ProgramHandle getProgram() const { return program; }
    bool isValid() const { return bgfx::isValid(program); }
    void destroy();

private:
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

    bgfx::ShaderHandle loadShader(const std::string& baseName);
    std::string selectShaderPath(const std::string& baseName);
};

} // namespace Engine

