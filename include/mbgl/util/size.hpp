#pragma once

#include <cstdint>

namespace mbgl {

class Size {
public:
    constexpr Size() = default;

    constexpr Size(const uint32_t width_, const uint32_t height_) noexcept
        : width(width_),
          height(height_) {}

    constexpr uint32_t area() const noexcept { return width * height; }

    constexpr float aspectRatio() const noexcept { return static_cast<float>(width) / static_cast<float>(height); }

    constexpr bool isEmpty() const noexcept { return width == 0 || height == 0; }

    uint32_t width = 0;
    uint32_t height = 0;
};

constexpr bool operator==(const Size& a, const Size& b) noexcept {
    return a.width == b.width && a.height == b.height;
}

constexpr bool operator!=(const Size& a, const Size& b) noexcept {
    return !(a == b);
}

} // namespace mbgl
