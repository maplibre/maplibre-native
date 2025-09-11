#pragma once

#include <mbgl/util/color.hpp>
#include <optional>

namespace mbgl {
namespace gfx {

class Renderable;

struct RenderPassDescriptor {
    Renderable& renderable;
    std::optional<Color> clearColor;
    std::optional<float> clearDepth;
    std::optional<int32_t> clearStencil;
};

} // namespace gfx
} // namespace mbgl