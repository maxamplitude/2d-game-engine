#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/types.h"

using namespace Engine;

TEST_CASE("Color toUint32 packing order", "[core][color]") {
    Color c(0x12, 0x34, 0x56, 0x78);
    REQUIRE(c.toUint32() == 0x12345678);
}

TEST_CASE("Color toVec4 normalized", "[core][color]") {
    Color c(128, 64, 32, 255);
    Vec4 v = c.toVec4();
    REQUIRE_THAT(v.x, Catch::Matchers::WithinAbs(128.0f / 255.0f, 1e-5f));
    REQUIRE_THAT(v.y, Catch::Matchers::WithinAbs(64.0f / 255.0f, 1e-5f));
    REQUIRE_THAT(v.z, Catch::Matchers::WithinAbs(32.0f / 255.0f, 1e-5f));
    REQUIRE_THAT(v.w, Catch::Matchers::WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("Color constants", "[core][color]") {
    REQUIRE(Color::White.toUint32() == 0xFFFFFFFF);
    REQUIRE(Color::Black.toUint32() == 0x000000FF);
    REQUIRE(Color::Transparent.toUint32() == 0x00000000);
    REQUIRE(Color::DarkBlue.toUint32() == 0x14143CFF);
}

