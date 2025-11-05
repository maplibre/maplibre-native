#pragma once

#include <cstdint>

namespace mbgl {
namespace gfx {

class ScissorRect {
public:
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;

    bool operator==(const ScissorRect& other) const = default;

    bool operator!=(const ScissorRect& other) const = default;
};

} // namespace gfx
} // namespace mbgl
