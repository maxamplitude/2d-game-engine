#pragma once
#include "scene.h"
#include <vector>
#include <functional>

namespace Engine {

class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager() = default;

    void changeScene(ScenePtr newScene);
    void pushScene(ScenePtr newScene);
    void popScene();

    void update(float dt);
    void handleInput(InputManager& input, float dt);
    void render(Renderer& renderer);

    bool hasActiveScene() const { return !sceneStack.empty(); }
    size_t getSceneCount() const { return sceneStack.size(); }

private:
    std::vector<ScenePtr> sceneStack;
};

} // namespace Engine

