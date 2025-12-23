#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "math/vector.h"

using namespace Engine;

TEST_CASE("toRadians and toDegrees are inverses", "[math][vector]") {
    float deg = 180.0f;
    float rad = toRadians(deg);
    REQUIRE(rad == Catch::Approx(3.14159265f).margin(1e-4f));
    REQUIRE(toDegrees(rad) == Catch::Approx(deg).margin(1e-4f));
}

TEST_CASE("Vec2 length and normalize", "[math][vector]") {
    Vec2 v(3.0f, 4.0f);
    REQUIRE(glm::length(v) == Catch::Approx(5.0f));
    Vec2 n = glm::normalize(v);
    REQUIRE(glm::length(n) == Catch::Approx(1.0f));
}

