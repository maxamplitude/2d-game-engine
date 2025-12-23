#pragma once
#include "math/vector.h"
#include <cstdint>

namespace Engine {

struct Color {
    uint8_t r{255}, g{255}, b{255}, a{255};

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}

    Vec4 toVec4() const {
        return Vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    uint32_t toUint32() const {
        return (static_cast<uint32_t>(r) << 24) |
               (static_cast<uint32_t>(g) << 16) |
               (static_cast<uint32_t>(b) << 8) |
               static_cast<uint32_t>(a);
    }

    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;
    static const Color DarkBlue;
};

} // namespace Engine

