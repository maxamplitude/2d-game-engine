#pragma once
#include <memory>

namespace Engine {

class Renderer;
class InputManager;

class Scene {
public:
    virtual ~Scene() = default;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onPause() {}
    virtual void onResume() {}

    virtual void handleInput(InputManager&, float) {}
    virtual void update(float dt) = 0;
    virtual void render(Renderer& renderer) = 0;

    void setManager(void* mgr) { manager = mgr; }

protected:
    Scene() = default;
    void* manager = nullptr; // opaque to avoid tight coupling
};

using ScenePtr = std::unique_ptr<Scene>;

} // namespace Engine

