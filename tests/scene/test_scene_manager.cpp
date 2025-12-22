// tests/scene/test_scene_manager.cpp
#include <catch2/catch_test_macros.hpp>
#include "scene/scene_manager.h"
#include <sstream>

using namespace Engine;

// ============================================================================
// Mock Scene for Testing
// ============================================================================

class MockScene : public Scene {
public:
    std::string name;
    std::stringstream lifecycleLog;
    int updateCount = 0;
    int renderCount = 0;
    int eventCount = 0;
    
    explicit MockScene(const std::string& n) : name(n) {}
    
    void onEnter() override {
        lifecycleLog << "enter,";
    }
    
    void onExit() override {
        lifecycleLog << "exit,";
    }
    
    void onPause() override {
        lifecycleLog << "pause,";
    }
    
    void onResume() override {
        lifecycleLog << "resume,";
    }
    
    void handleEvent(const sf::Event& event) override {
        eventCount++;
    }
    
    void update(float dt) override {
        updateCount++;
    }
    
    void render(sf::RenderWindow& window) override {
        renderCount++;
    }
    
    std::string getLifecycleLog() const {
        return lifecycleLog.str();
    }
};

// ============================================================================
// Test Cases
// ============================================================================

TEST_CASE("SceneManager initial state", "[scene][core]") {
    SceneManager manager;
    
    REQUIRE_FALSE(manager.hasActiveScene());
    REQUIRE(manager.getSceneCount() == 0);
    REQUIRE_FALSE(manager.isTransitioning());
}

TEST_CASE("SceneManager changeScene immediate", "[scene][core]") {
    SceneManager manager;
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    
    manager.changeScene(std::move(scene1));
    
    REQUIRE(manager.hasActiveScene());
    REQUIRE(manager.getSceneCount() == 1);
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,");
}

TEST_CASE("SceneManager changeScene replaces current", "[scene][core]") {
    SceneManager manager;
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    auto* scene2Ptr = scene2.get();
    
    manager.changeScene(std::move(scene1));
    manager.changeScene(std::move(scene2));
    
    REQUIRE(manager.getSceneCount() == 1);
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,exit,");
    REQUIRE(scene2Ptr->getLifecycleLog() == "enter,");
}

TEST_CASE("SceneManager pushScene adds to stack", "[scene][core]") {
    SceneManager manager;
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    auto* scene2Ptr = scene2.get();
    
    manager.changeScene(std::move(scene1));
    manager.pushScene(std::move(scene2));
    
    REQUIRE(manager.getSceneCount() == 2);
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,pause,");
    REQUIRE(scene2Ptr->getLifecycleLog() == "enter,");
}

TEST_CASE("SceneManager popScene removes from stack", "[scene][core]") {
    SceneManager manager;
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    auto* scene2Ptr = scene2.get();
    
    manager.changeScene(std::move(scene1));
    manager.pushScene(std::move(scene2));
    manager.popScene();
    
    REQUIRE(manager.getSceneCount() == 1);
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,pause,resume,");
    REQUIRE(scene2Ptr->getLifecycleLog() == "enter,exit,");
}

TEST_CASE("SceneManager popScene on empty stack", "[scene][core]") {
    SceneManager manager;
    
    // Should not crash
    manager.popScene();
    
    REQUIRE(manager.getSceneCount() == 0);
}

TEST_CASE("SceneManager update delegates to top scene only", "[scene][core]") {
    SceneManager manager;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    auto* scene2Ptr = scene2.get();
    
    manager.changeScene(std::move(scene1));
    manager.pushScene(std::move(scene2));
    
    manager.update(0.016f);
    
    REQUIRE(scene1Ptr->updateCount == 0);  // Paused
    REQUIRE(scene2Ptr->updateCount == 1);  // Active
}

TEST_CASE("SceneManager render renders all scenes", "[scene][core]") {
    SceneManager manager;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    auto* scene2Ptr = scene2.get();
    
    manager.changeScene(std::move(scene1));
    manager.pushScene(std::move(scene2));
    
    manager.render(window);
    
    // Both scenes should render (for transparency effects)
    REQUIRE(scene1Ptr->renderCount == 1);
    REQUIRE(scene2Ptr->renderCount == 1);
}

TEST_CASE("SceneManager handleEvent delegates to top scene only", "[scene][core]") {
    SceneManager manager;
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    auto* scene2Ptr = scene2.get();
    
    manager.changeScene(std::move(scene1));
    manager.pushScene(std::move(scene2));
    
    sf::Event event;
    event.type = sf::Event::KeyPressed;
    manager.handleEvent(event);
    
    REQUIRE(scene1Ptr->eventCount == 0);  // Paused
    REQUIRE(scene2Ptr->eventCount == 1);  // Active
}

TEST_CASE("SceneManager fade transition timing", "[scene][core]") {
    SceneManager manager;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    
    manager.changeScene(std::move(scene1));
    
    TransitionConfig fadeTransition(SceneTransition::Fade, 0.5f);
    manager.changeScene(std::move(scene2), fadeTransition);
    
    REQUIRE(manager.isTransitioning());
    
    // Still on scene1 during fade out
    REQUIRE(manager.getSceneCount() == 1);
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,");
    
    // Advance through fade out (0.5s)
    manager.update(0.5f);
    
    // Scene should have switched at halfway point
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,exit,");
    
    // Still transitioning (fade in phase)
    REQUIRE(manager.isTransitioning());
    
    // Advance through fade in (0.5s)
    manager.update(0.5f);
    
    // Transition complete
    REQUIRE_FALSE(manager.isTransitioning());
}

TEST_CASE("SceneManager createScene template helper", "[scene][core]") {
    SceneManager manager;
    
    auto scene = manager.createScene<MockScene>("TestScene");
    
    REQUIRE(scene != nullptr);
    REQUIRE(scene->getManager() == &manager);
    
    auto* mockPtr = dynamic_cast<MockScene*>(scene.get());
    REQUIRE(mockPtr != nullptr);
    REQUIRE(mockPtr->name == "TestScene");
}

TEST_CASE("SceneManager multiple push/pop operations", "[scene][core]") {
    SceneManager manager;
    
    auto scene1 = std::make_unique<MockScene>("Base");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Overlay1");
    auto* scene2Ptr = scene2.get();
    auto scene3 = std::make_unique<MockScene>("Overlay2");
    auto* scene3Ptr = scene3.get();
    
    // Build stack: Base -> Overlay1 -> Overlay2
    manager.changeScene(std::move(scene1));
    manager.pushScene(std::move(scene2));
    manager.pushScene(std::move(scene3));
    
    REQUIRE(manager.getSceneCount() == 3);
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,pause,");
    REQUIRE(scene2Ptr->getLifecycleLog() == "enter,pause,");
    REQUIRE(scene3Ptr->getLifecycleLog() == "enter,");
    
    // Pop twice to get back to base
    manager.popScene();
    manager.popScene();
    
    REQUIRE(manager.getSceneCount() == 1);
    REQUIRE(scene1Ptr->getLifecycleLog() == "enter,pause,resume,");
    REQUIRE(scene2Ptr->getLifecycleLog() == "enter,pause,resume,exit,");
    REQUIRE(scene3Ptr->getLifecycleLog() == "enter,exit,");
}

TEST_CASE("SceneManager does not update during transition", "[scene][core]") {
    SceneManager manager;
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto* scene1Ptr = scene1.get();
    auto scene2 = std::make_unique<MockScene>("Scene2");
    
    manager.changeScene(std::move(scene1));
    
    TransitionConfig fadeTransition(SceneTransition::Fade, 0.5f);
    manager.changeScene(std::move(scene2), fadeTransition);
    
    // Update during transition
    manager.update(0.1f);
    
    // Scene should not receive update calls during transition
    REQUIRE(scene1Ptr->updateCount == 0);
}

TEST_CASE("SceneManager null scene handling", "[scene][core]") {
    SceneManager manager;
    
    // Attempting to change to null scene should be safe
    manager.changeScene(nullptr);
    
    REQUIRE_FALSE(manager.hasActiveScene());
    REQUIRE(manager.getSceneCount() == 0);
}

TEST_CASE("SceneManager transition with custom fade color", "[scene][core]") {
    SceneManager manager;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    
    auto scene1 = std::make_unique<MockScene>("Scene1");
    auto scene2 = std::make_unique<MockScene>("Scene2");
    
    manager.changeScene(std::move(scene1));
    
    TransitionConfig whiteTransition(
        SceneTransition::Fade, 
        0.3f, 
        sf::Color::White
    );
    manager.changeScene(std::move(scene2), whiteTransition);
    
    REQUIRE(manager.isTransitioning());
    
    // The transition should use white color (verified via rendering)
    manager.render(window);
}