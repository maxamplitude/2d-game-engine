
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/transform.h"

using namespace Engine;

TEST_CASE("Transform default construction", "[transform][core]") {
    Transform t;
    
    REQUIRE(t.position.x == 0.0f);
    REQUIRE(t.position.y == 0.0f);
    REQUIRE(t.scale.x == 1.0f);
    REQUIRE(t.scale.y == 1.0f);
    REQUIRE(t.rotation == 0.0f);
}

TEST_CASE("Transform position translation", "[transform][core]") {
    Transform t;
    t.position = {100.0f, 50.0f};
    
    Vec2 point(10.0f, 5.0f);
    Vec2 transformed = t.transformPoint(point);
    
    REQUIRE_THAT(transformed.x, Catch::Matchers::WithinRel(110.0f, 0.01f));
    REQUIRE_THAT(transformed.y, Catch::Matchers::WithinRel(55.0f, 0.01f));
}

TEST_CASE("Transform composition", "[transform][core]") {
    Transform t1;
    t1.position = {10.0f, 20.0f};
    t1.scale = {2.0f, 2.0f};

    Transform t2;
    t2.position = {5.0f, 10.0f};
    t2.scale = {2.0f, 2.0f};
    t2.rotation = 45.0f;

    Transform combined = t1 * t2;

    REQUIRE(combined.position.x == 15.0f);
    REQUIRE(combined.position.y == 30.0f);
    REQUIRE(combined.scale.x == 4.0f);
    REQUIRE(combined.scale.y == 4.0f);
    REQUIRE(combined.rotation == 45.0f);
}

TEST_CASE("Transform equality comparison", "[transform][core]") {
    Transform t1;
    t1.position = {100.0f, 200.0f};
    t1.scale = {2.0f, 2.0f};
    
    Transform t2;
    t2.position = {100.0f, 200.0f};
    t2.scale = {2.0f, 2.0f};
    
    Transform t3;
    t3.position = {100.0f, 201.0f};
    
    REQUIRE(t1 == t2);
    REQUIRE(t1 != t3);
}

TEST_CASE("Transform to matrix", "[transform][core]") {
    Transform t;
    t.position = {50.0f, 100.0f};
    
    Mat4 mat = t.toMatrix();
    Vec4 transformed = mat * Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    REQUIRE_THAT(transformed.x, Catch::Matchers::WithinRel(50.0f, 0.01f));
    REQUIRE_THAT(transformed.y, Catch::Matchers::WithinRel(100.0f, 0.01f));
}