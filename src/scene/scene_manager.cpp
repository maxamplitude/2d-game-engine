#include "scene_manager.h"

namespace Engine {

void SceneManager::changeScene(ScenePtr newScene) {
    sceneStack.clear();
    if (newScene) {
        newScene->setManager(this);
        newScene->onEnter();
        sceneStack.push_back(std::move(newScene));
    }
}

void SceneManager::pushScene(ScenePtr newScene) {
    if (!newScene) return;
    if (!sceneStack.empty()) {
        sceneStack.back()->onPause();
    }
    newScene->setManager(this);
    newScene->onEnter();
    sceneStack.push_back(std::move(newScene));
}

void SceneManager::popScene() {
    if (sceneStack.empty()) return;
    sceneStack.back()->onExit();
    sceneStack.pop_back();
    if (!sceneStack.empty()) {
        sceneStack.back()->onResume();
    }
}

void SceneManager::handleInput(InputManager& input, float dt) {
    if (sceneStack.empty()) return;
    sceneStack.back()->handleInput(input, dt);
}

void SceneManager::update(float dt) {
    if (sceneStack.empty()) return;
    sceneStack.back()->update(dt);
}

void SceneManager::render(Renderer& renderer) {
    for (auto& scene : sceneStack) {
        scene->render(renderer);
    }
}

} // namespace Engine

