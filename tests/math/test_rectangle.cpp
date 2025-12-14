#include <catch2/catch_test_macros.hpp>
#include "math/rectangle.h"

using namespace Engine;

TEST_CASE("Rectangle default construction", "[rectangle][math]") {
    Rectangle rect;
    
    REQUIRE(rect.x == 0.0f);
    REQUIRE(rect.y == 0.0f);
    REQUIRE(rect.width == 0.0f);
    REQUIRE(rect.height == 0.0f);
    REQUIRE(rect.isEmpty());
}

TEST_CASE("Rectangle parameterized construction", "[rectangle][math]") {
    Rectangle rect(10.0f, 20.0f, 100.0f, 50.0f);
    
    REQUIRE(rect.x == 10.0f);
    REQUIRE(rect.y == 20.0f);
    REQUIRE(rect.width == 100.0f);
    REQUIRE(rect.height == 50.0f);
}

TEST_CASE("Rectangle boundary accessors", "[rectangle][math]") {
    Rectangle rect(10.0f, 20.0f, 100.0f, 50.0f);
    
    REQUIRE(rect.left() == 10.0f);
    REQUIRE(rect.right() == 110.0f);
    REQUIRE(rect.top() == 20.0f);
    REQUIRE(rect.bottom() == 70.0f);
}

TEST_CASE("Rectangle center calculation", "[rectangle][math]") {
    Rectangle rect(0.0f, 0.0f, 100.0f, 50.0f);
    sf::Vector2f center = rect.center();
    
    REQUIRE(center.x == 50.0f);
    REQUIRE(center.y == 25.0f);
}

TEST_CASE("Rectangle intersection detection", "[rectangle][math][collision]") {
    Rectangle rect1(0.0f, 0.0f, 100.0f, 100.0f);
    Rectangle rect2(50.0f, 50.0f, 100.0f, 100.0f);
    Rectangle rect3(200.0f, 200.0f, 50.0f, 50.0f);
    
    SECTION("Overlapping rectangles intersect") {
        REQUIRE(rect1.intersects(rect2));
        REQUIRE(rect2.intersects(rect1));  // Commutative
    }
    
    SECTION("Non-overlapping rectangles don't intersect") {
        REQUIRE_FALSE(rect1.intersects(rect3));
        REQUIRE_FALSE(rect3.intersects(rect1));
    }
    
    SECTION("Rectangle intersects with itself") {
        REQUIRE(rect1.intersects(rect1));
    }
}

TEST_CASE("Rectangle contains point", "[rectangle][math]") {
    Rectangle rect(10.0f, 10.0f, 50.0f, 50.0f);
    
    SECTION("Point inside rectangle") {
        REQUIRE(rect.contains({30.0f, 30.0f}));
    }
    
    SECTION("Point on boundary") {
        REQUIRE(rect.contains({10.0f, 10.0f}));
        REQUIRE(rect.contains({60.0f, 60.0f}));
    }
    
    SECTION("Point outside rectangle") {
        REQUIRE_FALSE(rect.contains({100.0f, 100.0f}));
        REQUIRE_FALSE(rect.contains({5.0f, 5.0f}));
    }
}

TEST_CASE("Rectangle intersection calculation", "[rectangle][math][collision]") {
    Rectangle rect1(0.0f, 0.0f, 100.0f, 100.0f);
    Rectangle rect2(50.0f, 50.0f, 100.0f, 100.0f);
    
    Rectangle intersection = rect1.getIntersection(rect2);
    
    REQUIRE(intersection.x == 50.0f);
    REQUIRE(intersection.y == 50.0f);
    REQUIRE(intersection.width == 50.0f);
    REQUIRE(intersection.height == 50.0f);
}

TEST_CASE("Rectangle intersection returns empty for non-overlapping", "[rectangle][math][collision]") {
    Rectangle rect1(0.0f, 0.0f, 50.0f, 50.0f);
    Rectangle rect2(100.0f, 100.0f, 50.0f, 50.0f);
    
    Rectangle intersection = rect1.getIntersection(rect2);
    
    REQUIRE(intersection.isEmpty());
}

TEST_CASE("Rectangle area calculation", "[rectangle][math]") {
    Rectangle rect(0.0f, 0.0f, 10.0f, 20.0f);
    
    REQUIRE(rect.area() == 200.0f);
}