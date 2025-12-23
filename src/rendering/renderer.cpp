#include "renderer.h"
#include "platform/logging.h"
#include <GLFW/glfw3.h>
#if defined(__linux__) && !defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>
#include <dlfcn.h>
#endif

namespace Engine {

namespace {
#if defined(__linux__) && !defined(__APPLE__)
using WaylandDisplayFn = void* (*)();
using WaylandWindowFn = void* (*)(GLFWwindow*);

struct WaylandSymbols {
    WaylandDisplayFn getDisplay{nullptr};
    WaylandWindowFn getWindow{nullptr};
    bool loaded{false};
};

WaylandSymbols loadWaylandSymbols() {
    WaylandSymbols symbols;
    void* handle = dlopen("libglfw.so.3", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        handle = dlopen("libglfw.so", RTLD_LAZY | RTLD_LOCAL);
    }
    if (!handle) {
        return symbols;
    }
    symbols.getDisplay = reinterpret_cast<WaylandDisplayFn>(dlsym(handle, "glfwGetWaylandDisplay"));
    symbols.getWindow = reinterpret_cast<WaylandWindowFn>(dlsym(handle, "glfwGetWaylandWindow"));
    symbols.loaded = symbols.getDisplay && symbols.getWindow;
    return symbols;
}
#endif
} // namespace

static bgfx::RendererType::Enum toBgfxBackend(RendererBackend backend) {
    switch (backend) {
        case RendererBackend::OpenGL: return bgfx::RendererType::OpenGL;
        case RendererBackend::Vulkan: return bgfx::RendererType::Vulkan;
        case RendererBackend::Metal: return bgfx::RendererType::Metal;
        case RendererBackend::Direct3D11: return bgfx::RendererType::Direct3D11;
        case RendererBackend::Auto:
        default:
#if defined(__linux__) || defined(__APPLE__)
            return bgfx::RendererType::OpenGL;
#else
            return bgfx::RendererType::Count;
#endif
    }
}

Renderer::Renderer(Window* window, const RendererConfig& config)
    : window(window), debugEnabled(config.debug) {
    bgfx::PlatformData pd{};
#ifdef _WIN32
    pd.nwh = window->getNativeWindowHandle();
#elif __APPLE__
    pd.nwh = window->getNativeWindowHandle();
#else
    // Prefer X11, fallback to Wayland if X11 unavailable.
    pd.ndt = glfwGetX11Display();
    pd.nwh = window->getNativeWindowHandle();
#ifdef GLFW_EXPOSE_NATIVE_WAYLAND
    if (pd.nwh == nullptr || pd.ndt == nullptr) {
        static WaylandSymbols wl = loadWaylandSymbols();
        if (wl.loaded) {
            auto* wlDisplay = wl.getDisplay();
            auto* wlWindow = wl.getWindow(static_cast<GLFWwindow*>(window->getNativeHandle()));
            if (wlDisplay && wlWindow) {
                pd.ndt = wlDisplay;
                pd.nwh = wlWindow;
            }
        }
    }
#endif
    if (pd.nwh == nullptr || pd.ndt == nullptr) {
        Log::critical("Failed to obtain native window/display handles for BGFX");
        initialized = false;
        return;
    }
#endif

    bgfx::Init init;
    init.platformData = pd;
    init.type = toBgfxBackend(config.backend);
    init.resolution.width = window->getWidth();
    init.resolution.height = window->getHeight();
    init.resolution.reset = config.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
    init.vendorId = BGFX_PCI_ID_NONE;

    if (!bgfx::init(init)) {
        Log::critical("Failed to initialize BGFX (ndt={}, nwh={})", static_cast<void*>(pd.ndt), pd.nwh);
        initialized = false;
        return;
    }

    initialized = true;
    Log::info("BGFX initialized: {}", bgfx::getRendererName(bgfx::getRendererType()));

    resetFlags = config.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
    if (debugEnabled) {
        bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, Color::Black.toUint32(), 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, window->getWidth(), window->getHeight());

    window->setResizeCallback([this](int w, int h) { resize(w, h); });
}

Renderer::~Renderer() {
    if (initialized) {
        bgfx::shutdown();
        Log::info("BGFX shutdown");
    }
}

void Renderer::beginFrame() {
    if (!initialized) return;
    bgfx::touch(0);
}

void Renderer::endFrame() {
    if (!initialized) return;
    bgfx::frame();
}

void Renderer::clear(const Color& color) {
    if (!initialized) return;
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, color.toUint32(), 1.0f, 0);
}

bgfx::RendererType::Enum Renderer::getBackend() const {
    return bgfx::getRendererType();
}

const char* Renderer::getBackendName() const {
    return bgfx::getRendererName(getBackend());
}

void Renderer::resize(int width, int height) {
    if (!initialized) return;
    bgfx::reset(width, height, resetFlags);
    bgfx::setViewRect(0, 0, 0, width, height);
    Log::info("Renderer resized: {}x{}", width, height);
}

} // namespace Engine

