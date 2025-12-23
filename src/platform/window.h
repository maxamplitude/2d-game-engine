#pragma once
#include "math/vector.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <string>

namespace Engine {

struct WindowConfig {
    int width = 800;
    int height = 600;
    std::string title = "Engine";
    bool vsync = true;
    bool resizable = true;
};

class Window {
public:
    explicit Window(const WindowConfig& config);
    ~Window();

    bool isOpen() const;
    void close();
    void pollEvents();

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Vec2 getSize() const { return Vec2(static_cast<float>(width), static_cast<float>(height)); }
    float getAspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }

    void setVSync(bool enabled);
    bool isVSyncEnabled() const { return vsyncEnabled; }

    GLFWwindow* getNativeHandle() const { return window; }
    void* getNativeWindowHandle() const;

    void setResizeCallback(std::function<void(int, int)> callback);
    void setCloseCallback(std::function<void()> callback);

private:
    GLFWwindow* window = nullptr;
    int width{};
    int height{};
    bool vsyncEnabled{};

    std::function<void(int, int)> resizeCallback;
    std::function<void()> closeCallback;

    static void glfwResizeCallback(GLFWwindow* window, int width, int height);
    static void glfwCloseCallback(GLFWwindow* window);
};

} // namespace Engine

