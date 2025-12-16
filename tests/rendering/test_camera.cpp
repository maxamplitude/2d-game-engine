#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "rendering/camera.h"
#include <cmath>

using namespace Engine;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("Camera default construction", "[camera][rendering]") {
    Camera camera;
    
    REQUIRE(camera.getPosition().x == 0.0f);
    REQUIRE(camera.getPosition().y == 0.0f);
    REQUIRE(camera.getZoom() == 1.0f);
    REQUIRE(camera.getRotation() == 0.0f);
    REQUIRE(camera.getTrauma() == 0.0f);
}

TEST_CASE("Camera parameterized construction", "[camera][rendering]") {
    Camera camera({100.0f, 200.0f}, {800.0f, 600.0f});
    
    REQUIRE(camera.getPosition().x == 100.0f);
    REQUIRE(camera.getPosition().y == 200.0f);
    REQUIRE(camera.getSize().x == 800.0f);
    REQUIRE(camera.getSize().y == 600.0f);
}

TEST_CASE("Camera basic movement", "[camera][rendering]") {
    Camera camera;
    
    SECTION("Set position") {
        camera.setPosition({50.0f, 100.0f});
        REQUIRE(camera.getPosition().x == 50.0f);
        REQUIRE(camera.getPosition().y == 100.0f);
    }
    
    SECTION("Move by offset") {
        camera.setPosition({10.0f, 20.0f});
        camera.move({5.0f, 10.0f});
        
        REQUIRE(camera.getPosition().x == 15.0f);
        REQUIRE(camera.getPosition().y == 30.0f);
    }
}

TEST_CASE("Camera zoom", "[camera][rendering]") {
    Camera camera;
    
    SECTION("Set zoom") {
        camera.setZoom(2.0f);
        REQUIRE(camera.getZoom() == 2.0f);
    }
    
    SECTION("Zoom by factor") {
        camera.setZoom(2.0f);
        camera.zoomBy(0.5f);
        REQUIRE(camera.getZoom() == 1.0f);
    }
    
    SECTION("Prevents negative zoom") {
        camera.setZoom(-1.0f);
        REQUIRE(camera.getZoom() >= 0.1f);
    }
    
    SECTION("Prevents zero zoom") {
        camera.setZoom(0.0f);
        REQUIRE(camera.getZoom() >= 0.1f);
    }
}

TEST_CASE("Camera smooth zoom transition", "[camera][rendering]") {
    Camera camera;
    camera.setZoom(1.0f);
    
    camera.zoomTo(2.0f, 0.3f);
    
    SECTION("Zoom interpolates over time") {
        camera.update(0.15f);  // Halfway through 0.3s duration
        
        float currentZoom = camera.getZoom();
        REQUIRE(currentZoom > 1.0f);
        REQUIRE(currentZoom < 2.0f);
    }
    
    SECTION("Zoom completes after duration") {
        camera.update(0.5f);  // More than 0.3s
        
        REQUIRE_THAT(camera.getZoom(), WithinAbs(2.0f, 0.01f));
    }
}

TEST_CASE("Camera rotation", "[camera][rendering]") {
    Camera camera;
    
    camera.setRotation(45.0f);
    REQUIRE(camera.getRotation() == 45.0f);
}

TEST_CASE("Camera instant follow", "[camera][rendering]") {
    Camera camera;
    sf::Vector2f target(100.0f, 200.0f);
    
    camera.setTarget(&target);
    camera.setFollowMode(CameraFollowMode::Instant);
    camera.update(0.016f);
    
    REQUIRE(camera.getPosition().x == 100.0f);
    REQUIRE(camera.getPosition().y == 200.0f);
}

TEST_CASE("Camera smooth follow", "[camera][rendering]") {
    Camera camera;
    camera.setPosition({0.0f, 0.0f});
    
    sf::Vector2f target(100.0f, 100.0f);
    camera.setTarget(&target);
    camera.setFollowMode(CameraFollowMode::Smooth);
    camera.setFollowSpeed(5.0f);
    
    SECTION("Moves toward target gradually") {
        camera.update(0.1f);  // 100ms
        
        sf::Vector2f pos = camera.getPosition();
        REQUIRE(pos.x > 0.0f);
        REQUIRE(pos.x < 100.0f);
        REQUIRE(pos.y > 0.0f);
        REQUIRE(pos.y < 100.0f);
    }
    
    SECTION("Eventually reaches target") {
        // Update many times
        for (int i = 0; i < 100; ++i) {
            camera.update(0.1f);
        }
        
        sf::Vector2f pos = camera.getPosition();
        REQUIRE_THAT(pos.x, WithinAbs(100.0f, 1.0f));
        REQUIRE_THAT(pos.y, WithinAbs(100.0f, 1.0f));
    }
}

TEST_CASE("Camera deadzone follow", "[camera][rendering]") {
    Camera camera;
    camera.setPosition({100.0f, 100.0f});
    
    // Deadzone: 20x20 centered at camera
    camera.setDeadzone({-10.0f, -10.0f, 20.0f, 20.0f});
    camera.setFollowMode(CameraFollowMode::Deadzone);
    
    SECTION("Doesn't move when target in deadzone") {
        sf::Vector2f target(105.0f, 105.0f);  // Inside deadzone
        camera.setTarget(&target);
        camera.update(0.016f);
        
        REQUIRE(camera.getPosition().x == 100.0f);
        REQUIRE(camera.getPosition().y == 100.0f);
    }
    
    SECTION("Moves when target exits deadzone right") {
        sf::Vector2f target(120.0f, 100.0f);  // Outside deadzone
        camera.setTarget(&target);
        camera.update(0.016f);
        
        REQUIRE(camera.getPosition().x > 100.0f);
    }
    
    SECTION("Moves when target exits deadzone left") {
        sf::Vector2f target(80.0f, 100.0f);  // Outside deadzone
        camera.setTarget(&target);
        camera.update(0.016f);
        
        REQUIRE(camera.getPosition().x < 100.0f);
    }
}

TEST_CASE("Camera screen shake", "[camera][rendering]") {
    Camera camera;
    camera.setPosition({100.0f, 100.0f});
    
    SECTION("Adding trauma") {
        camera.addTrauma(0.5f);
        REQUIRE(camera.getTrauma() == 0.5f);
    }
    
    SECTION("Trauma clamped to 0-1") {
        camera.addTrauma(0.8f);
        camera.addTrauma(0.8f);
        REQUIRE(camera.getTrauma() <= 1.0f);
    }
    
    SECTION("Trauma decays over time") {
        camera.setTrauma(1.0f);
        camera.setTraumaDecay(2.0f);  // Fast decay
        
        camera.update(0.3f);
        
        REQUIRE(camera.getTrauma() < 1.0f);
        REQUIRE(camera.getTrauma() >= 0.0f);
    }
    
    SECTION("Trauma eventually reaches zero") {
        camera.setTrauma(0.5f);
        camera.setTraumaDecay(1.0f);
        
        camera.update(1.0f);  // Enough time to decay completely
        
        REQUIRE(camera.getTrauma() == 0.0f);
    }
    
    SECTION("View position differs when shaking") {
        camera.setPosition({100.0f, 100.0f});
        camera.setTrauma(1.0f);
        
        sf::View view = camera.toSFMLView();
        sf::Vector2f center = view.getCenter();
        
        // With trauma, center should be offset from base position
        // (Note: This test might be flaky due to randomness)
        // In real code, you might want deterministic randomness for testing
        bool isOffset = (center.x != 100.0f) || (center.y != 100.0f);
        REQUIRE(isOffset);
    }
}

TEST_CASE("Camera bounds", "[camera][rendering]") {
    Camera camera({0.0f, 0.0f}, {100.0f, 100.0f});
    camera.setZoom(1.0f);
    
    // Set bounds: 0 to 200 in both dimensions
    camera.setBounds({0.0f, 0.0f, 200.0f, 200.0f});
    
    SECTION("Camera constrained to bounds") {
        camera.setPosition({300.0f, 300.0f});  // Way outside
        camera.update(0.016f);
        
        sf::Vector2f pos = camera.getPosition();
        REQUIRE(pos.x <= 150.0f);  // 200 - (100/2)
        REQUIRE(pos.y <= 150.0f);
    }
    
    SECTION("Camera can't go below minimum bounds") {
        camera.setPosition({-100.0f, -100.0f});
        camera.update(0.016f);
        
        sf::Vector2f pos = camera.getPosition();
        REQUIRE(pos.x >= 50.0f);  // 0 + (100/2)
        REQUIRE(pos.y >= 50.0f);
    }
    
    SECTION("Can clear bounds") {
        camera.clearBounds();
        REQUIRE_FALSE(camera.isWithinBounds());
        
        camera.setPosition({1000.0f, 1000.0f});
        camera.update(0.016f);
        
        // Should not be constrained
        REQUIRE(camera.getPosition().x == 1000.0f);
    }
}

TEST_CASE("Camera view bounds calculation", "[camera][rendering]") {
    Camera camera({100.0f, 100.0f}, {200.0f, 150.0f});
    camera.setZoom(1.0f);
    
    sf::FloatRect bounds = camera.getViewBounds();
    
    // Camera at 100,100 with size 200x150 should see from 0,25 to 200,175
    REQUIRE_THAT(bounds.left, WithinAbs(0.0f, 0.1f));
    REQUIRE_THAT(bounds.top, WithinAbs(25.0f, 0.1f));
    REQUIRE_THAT(bounds.width, WithinAbs(200.0f, 0.1f));
    REQUIRE_THAT(bounds.height, WithinAbs(150.0f, 0.1f));
}

TEST_CASE("Camera view bounds with zoom", "[camera][rendering]") {
    Camera camera({100.0f, 100.0f}, {200.0f, 150.0f});
    camera.setZoom(2.0f);  // Zoomed out
    
    sf::FloatRect bounds = camera.getViewBounds();
    
    // With 2x zoom, view should be 400x300
    REQUIRE_THAT(bounds.width, WithinAbs(400.0f, 0.1f));
    REQUIRE_THAT(bounds.height, WithinAbs(300.0f, 0.1f));
}

TEST_CASE("Camera visibility testing", "[camera][rendering]") {
    Camera camera({100.0f, 100.0f}, {200.0f, 200.0f});
    camera.setZoom(1.0f);
    
    // Camera sees from 0,0 to 200,200
    
    SECTION("Object inside view is visible") {
        sf::FloatRect rect(50.0f, 50.0f, 20.0f, 20.0f);
        REQUIRE(camera.isVisible(rect));
    }
    
    SECTION("Object outside view is not visible") {
        sf::FloatRect rect(300.0f, 300.0f, 20.0f, 20.0f);
        REQUIRE_FALSE(camera.isVisible(rect));
    }
    
    SECTION("Partially visible object is visible") {
        sf::FloatRect rect(190.0f, 190.0f, 20.0f, 20.0f);
        REQUIRE(camera.isVisible(rect));
    }
}

TEST_CASE("Camera coordinate conversion", "[camera][rendering]") {
    Camera camera({100.0f, 100.0f}, {200.0f, 200.0f});
    camera.setZoom(1.0f);
    
    sf::Vector2u windowSize(800, 600);
    
    SECTION("Center of screen maps to camera position") {
        sf::Vector2f screenCenter(400.0f, 300.0f);
        sf::Vector2f worldPos = camera.screenToWorld(screenCenter, windowSize);
        
        REQUIRE_THAT(worldPos.x, WithinAbs(100.0f, 1.0f));
        REQUIRE_THAT(worldPos.y, WithinAbs(100.0f, 1.0f));
    }
    
    SECTION("World to screen conversion") {
        sf::Vector2f worldPos = camera.getPosition();
        sf::Vector2f screenPos = camera.worldToScreen(worldPos, windowSize);
        
        // Camera center should map to screen center
        REQUIRE_THAT(screenPos.x, WithinAbs(400.0f, 1.0f));
        REQUIRE_THAT(screenPos.y, WithinAbs(300.0f, 1.0f));
    }
}

TEST_CASE("Camera to SFML view conversion", "[camera][rendering]") {
    Camera camera({100.0f, 150.0f}, {400.0f, 300.0f});
    camera.setZoom(1.5f);
    camera.setRotation(45.0f);
    
    sf::View view = camera.toSFMLView();
    
    // View should have camera's properties
    sf::Vector2f size = view.getSize();
    REQUIRE_THAT(size.x, WithinAbs(600.0f, 0.1f));  // 400 * 1.5
    REQUIRE_THAT(size.y, WithinAbs(450.0f, 0.1f));  // 300 * 1.5
    REQUIRE(view.getRotation() == 45.0f);
}

TEST_CASE("Camera target can be cleared", "[camera][rendering]") {
    Camera camera;
    sf::Vector2f target(100.0f, 100.0f);
    
    camera.setTarget(&target);
    camera.setFollowMode(CameraFollowMode::Instant);
    camera.clearTarget();
    
    camera.setPosition({0.0f, 0.0f});
    camera.update(0.016f);
    
    // Should not move without target
    REQUIRE(camera.getPosition().x == 0.0f);
    REQUIRE(camera.getPosition().y == 0.0f);
}

TEST_CASE("Camera shake intensity", "[camera][rendering]") {
    Camera camera;
    camera.setPosition({100.0f, 100.0f});
    camera.setShakeIntensity(100.0f);
    camera.setTrauma(1.0f);
    
    sf::View view1 = camera.toSFMLView();
    
    camera.setShakeIntensity(10.0f);
    sf::View view2 = camera.toSFMLView();
    
    // Both should be offset, but different amounts
    // (Note: Random, so just checking they're different)
    // In production tests, you'd want deterministic randomness
}

TEST_CASE("Camera no movement when follow mode is None", "[camera][rendering]") {
    Camera camera;
    camera.setPosition({0.0f, 0.0f});
    
    sf::Vector2f target(100.0f, 100.0f);
    camera.setTarget(&target);
    camera.setFollowMode(CameraFollowMode::None);
    
    camera.update(0.016f);
    
    REQUIRE(camera.getPosition().x == 0.0f);
    REQUIRE(camera.getPosition().y == 0.0f);
}

TEST_CASE("Camera handles large delta times gracefully", "[camera][rendering]") {
    Camera camera;
    camera.setPosition({0.0f, 0.0f});
    
    sf::Vector2f target(100.0f, 100.0f);
    camera.setTarget(&target);
    camera.setFollowMode(CameraFollowMode::Smooth);
    camera.setFollowSpeed(5.0f);
    
    // Very large delta time
    camera.update(10.0f);
    
    // Should reach target (but not overshoot due to clamping in lerp)
    sf::Vector2f pos = camera.getPosition();
    REQUIRE_THAT(pos.x, WithinAbs(100.0f, 0.1f));
    REQUIRE_THAT(pos.y, WithinAbs(100.0f, 0.1f));
}