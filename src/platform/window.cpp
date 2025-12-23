#include "window.h"
#include "logging.h"

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif __APPLE__
    #define GLFW_EXPOSE_NATIVE_COCOA
#else
    #define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

namespace Engine {

Window::Window(const WindowConfig& config)
    : width(config.width), height(config.height), vsyncEnabled(config.vsync) {
    if (!glfwInit()) {
        Log::critical("Failed to initialize GLFW");
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    window = glfwCreateWindow(width, height, config.title.c_str(), nullptr, nullptr);
    if (!window) {
        Log::critical("Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, glfwResizeCallback);
    glfwSetWindowCloseCallback(window, glfwCloseCallback);

    Log::info("Window created: {}x{}", width, height);
}

Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool Window::isOpen() const {
    return window && !glfwWindowShouldClose(window);
}

void Window::close() {
    if (window) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::setVSync(bool enabled) {
    vsyncEnabled = enabled;
}

void* Window::getNativeWindowHandle() const {
#ifdef _WIN32
    return glfwGetWin32Window(window);
#elif __APPLE__
    return glfwGetCocoaWindow(window);
#else
    return (void*)(uintptr_t)glfwGetX11Window(window);
#endif
}

void Window::setResizeCallback(std::function<void(int, int)> callback) {
    resizeCallback = callback;
}

void Window::setCloseCallback(std::function<void()> callback) {
    closeCallback = callback;
}

void Window::glfwResizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->width = width;
    self->height = height;
    if (self->resizeCallback) {
        self->resizeCallback(width, height);
    }
}

void Window::glfwCloseCallback(GLFWwindow* window) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self->closeCallback) {
        self->closeCallback();
    }
}

} // namespace Engine

