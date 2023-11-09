#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) ClipUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ std::uint32_t stencil_ref;
    /* 68 */ std::uint32_t pad1, pad2, pad3;
    /* 80 */
};
static_assert(sizeof(ClipUBO) == 5 * 16);

} // namespace shaders
} // namespace mbgl
