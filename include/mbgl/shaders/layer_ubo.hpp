#pragma once

#include <mbgl/util/color.hpp>

#include <array>
#include <cstdint>

namespace mbgl {
namespace shaders {

enum class AttributeSource : int32_t {
    Constant,
    PerVertex,
    Computed,
};

enum class ExpressionFunction : int32_t {
    Constant,
    Linear,
    Exponential,
};

struct Expression {
    /* 0 */ ExpressionFunction function;
    /* 4 */
};
static_assert(sizeof(Expression) == 4);

struct Attribute {
    /* 0 */ AttributeSource source;
    /* 4 */ Expression expression;
    /* 8 */
};
static_assert(sizeof(Attribute) == 8);

//
// Global UBOs

struct alignas(16) GlobalPaintParamsUBO {
    std::array<float, 2> pattern_atlas_texsize;
    std::array<float, 2> units_to_pixels;
    std::array<float, 2> world_size;
    float camera_to_center_distance;
    float symbol_fade_change;
    float aspect_ratio;
    float pixel_ratio;
    float zoom;
    float pad1;
};
static_assert(sizeof(GlobalPaintParamsUBO) == 3 * 16);

enum {
    idGlobalPaintParamsUBO,
    globalUBOCount
};

} // namespace shaders
} // namespace mbgl
