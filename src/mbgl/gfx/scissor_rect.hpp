#pragma once

#include <cstdint>

namespace mbgl {
namespace gfx {

class ScissorRect {
public:
    int32_t x, y;
    uint32_t width, height;

    bool operator==(const ScissorRect& other) const {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    bool operator!=(const ScissorRect& other) const { return !(*this == other); }
};

} // namespace gfx
} // namespace mbgl
