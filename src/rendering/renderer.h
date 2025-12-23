#pragma once
#include "math/vector.h"
#include "core/types.h"
#include "platform/window.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace Engine {

enum class RendererBackend {
    Auto,
    OpenGL,
    Vulkan,
    Metal,
    Direct3D11
};

struct RendererConfig {
    RendererBackend backend = RendererBackend::Auto;
    bool vsync = true;
    bool debug = false;
};

class Renderer {
public:
    Renderer(Window* window, const RendererConfig& config);
    ~Renderer();

    void beginFrame();
    void endFrame();
    void clear(const Color& color = Color::Black);

    bgfx::RendererType::Enum getBackend() const;
    const char* getBackendName() const;

    int width() const { return window->getWidth(); }
    int height() const { return window->getHeight(); }
    bool isInitialized() const { return initialized; }

private:
    Window* window;
    bool debugEnabled;
    uint16_t resetFlags;
    bool initialized = false;

    void resize(int width, int height);
};

} // namespace Engine

