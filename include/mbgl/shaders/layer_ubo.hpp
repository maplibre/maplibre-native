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
    /*  0 */ std::array<float, 2> pattern_atlas_texsize;
    /*  8 */ std::array<float, 2> units_to_pixels;
    /* 16 */ std::array<float, 2> world_size;
    /* 24 */ float camera_to_center_distance;
    /* 28 */ float symbol_fade_change;
    /* 32 */ float aspect_ratio;
    /* 36 */ float pixel_ratio;
    /* 40 */ float map_zoom;
    /* 44 */ float pad1;
    /* 48 */
};
static_assert(sizeof(GlobalPaintParamsUBO) == 3 * 16);

#if MLN_RENDER_BACKEND_VULKAN
struct alignas(16) GlobalPlatformParamsUBO {
    /*  0 */ alignas(16) std::array<float, 2> rotation0;
    /* 16 */ alignas(16) std::array<float, 2> rotation1;
    /* 32 */
};
static_assert(sizeof(GlobalPlatformParamsUBO) == 2 * 16);
#endif

enum {
    idGlobalPaintParamsUBO,
#if MLN_RENDER_BACKEND_METAL
    idGlobalUBOIndex,
#elif MLN_RENDER_BACKEND_VULKAN
    idGlobalPlatformParamsUBO,
#elif MLN_RENDER_BACKEND_WEBGPU
    idGlobalUBOIndex,
#endif
    globalUBOCount
};

enum {
    idDrawableReservedVertexOnlyUBO = globalUBOCount,
    idDrawableReservedFragmentOnlyUBO,
    drawableReservedUBOCount
};

#define MLN_UBO_CONSOLIDATION (MLN_RENDER_BACKEND_METAL || MLN_RENDER_BACKEND_VULKAN || MLN_RENDER_BACKEND_WEBGPU)

} // namespace shaders
} // namespace mbgl
