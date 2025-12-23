#include "platform/platform.h"
#include "platform/logging.h"
#include "platform/window.h"
#include "rendering/renderer.h"
#include "rendering/quad_renderer.h"
#include "core/time_manager.h"
#include "core/types.h"
#include "rendering/camera.h"
#include "rendering/texture.h"
#include "scene/scene_manager.h"
#include "scene/scene.h"
#include "input/input_manager.h"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;

int main() {
    Platform::init();
    Log::init();

    WindowConfig windowConfig;
    windowConfig.width = 800;
    windowConfig.height = 600;
    windowConfig.title = "BGFX + GLFW Engine";
    Window window(windowConfig);

    RendererConfig rendererConfig;
    rendererConfig.backend = RendererBackend::Auto;
    rendererConfig.vsync = true;
    rendererConfig.debug = true;
    Renderer renderer(&window, rendererConfig);
    if (!renderer.isInitialized()) {
        Log::critical("Renderer failed to initialize. Exiting.");
        return 1;
    }

    QuadRenderer quadRenderer;
    quadRenderer.init();

    TimeManager time;
    InputManager input(window.getNativeHandle());

    // Simple demo scene renders a spinning quad
    class DemoScene : public Scene {
    public:
        DemoScene(Texture* tex, QuadRenderer* renderer, Camera* cam)
            : texture(tex), quad(renderer), camera(cam) {}

        void update(float dt) override {
            angle += dt;
        }

        void handleInput(InputManager& input, float dt) override {
            float move = 200.0f * dt;
            if (input.isActionActive("move_left")) camPos.x -= move;
            if (input.isActionActive("move_right")) camPos.x += move;
            if (input.isActionActive("move_up")) camPos.y -= move;
            if (input.isActionActive("move_down")) camPos.y += move;
            camera->setPosition(camPos);
        }

        void render(Renderer& renderer) override {
            Mat4 view = camera->getViewMatrix();
            Mat4 proj = camera->getProjection(static_cast<float>(renderer.width()),
                                              static_cast<float>(renderer.height()));
            Mat4 viewProj = proj * view;

            Mat4 model = glm::translate(glm::mat4(1.0f), Vec3(200.0f, 150.0f, 0.0f));
            model = glm::translate(model, Vec3(64.0f, 64.0f, 0.0f));
            model = glm::rotate(model, angle, Vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, Vec3(-64.0f, -64.0f, 0.0f));
            model = glm::scale(model, Vec3(128.0f, 128.0f, 1.0f));

            if (texture && texture->isValid()) {
                quad->draw(viewProj, model, texture->getHandle(), Color::White);
            }
        }

    private:
        Texture* texture;
        QuadRenderer* quad;
        Camera* camera;
        Vec2 camPos{0.0f, 0.0f};
        float angle = 0.0f;
    };

    Log::info("Backend: {}", renderer.getBackendName());

    // Create a tiny checkerboard texture
    Texture checkerTex;
    {
        const uint32_t w = 64, h = 64;
        std::vector<uint8_t> pixels(w * h * 4);
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                bool dark = ((x / 8) + (y / 8)) % 2 == 0;
                uint8_t c = dark ? 40 : 200;
                size_t idx = (y * w + x) * 4;
                pixels[idx + 0] = c;
                pixels[idx + 1] = c;
                pixels[idx + 2] = c;
                pixels[idx + 3] = 255;
            }
        }
        checkerTex.loadFromMemory(pixels.data(), static_cast<uint32_t>(pixels.size()));
    }

    Camera camera(Vec2(0.0f, 0.0f), Vec2(static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight())));
    camera.setZoom(1.0f);

    SceneManager scenes;
    scenes.changeScene(std::make_unique<DemoScene>(&checkerTex, &quadRenderer, &camera));

    while (window.isOpen()) {
        time.update();
        input.beginFrame();
        window.pollEvents();
        input.update(time.getDeltaTime());

        scenes.handleInput(input, time.getDeltaTime());
        scenes.update(time.getDeltaTime());

        renderer.beginFrame();
        renderer.clear(Color::Blue);
        scenes.render(renderer);
        renderer.endFrame();
    }

    Log::info("Shutting down...");
    quadRenderer.shutdown();
    Log::shutdown();
    Platform::shutdown();
    return 0;
}