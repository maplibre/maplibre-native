#pragma once

#include <mln/util/color.hpp>

#include <array>
#include <cstdint>

namespace mln {
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

/// Zoom fade table for `line-fade-zoom` and `fill-fade-zoom`, shared by all line and fill layers.
/// Entry `i` is the current opacity of the features whose fade zoom rounds to `i / fadeZoomStepsPerZoom`:
/// it moves towards 1 while the map's zoom is at or above that value and towards 0 below it.
constexpr std::size_t fadeZoomStepsPerZoom = 16;
constexpr std::size_t fadeZoomTableSize = 26 * fadeZoomStepsPerZoom; // zoom 0 to 25.9375

struct alignas(16) FadeZoomTableUBO {
    /* 0 */ std::array<float, fadeZoomTableSize> opacity;
    /* 1664 */
};
static_assert(sizeof(FadeZoomTableUBO) == fadeZoomTableSize * 4);
static_assert(sizeof(FadeZoomTableUBO) % 16 == 0);

#if MLN_RENDER_BACKEND_VULKAN
struct alignas(16) GlobalPlatformParamsUBO {
    /*  0 */ std::array<float, 4> surfaceRotation;
    /* 16 */
};
static_assert(sizeof(GlobalPlatformParamsUBO) == 16);
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

#define MLN_UBO_CONSOLIDATION (MLN_RENDER_BACKEND_METAL || MLN_RENDER_BACKEND_VULKAN || MLN_RENDER_BACKEND_WEBGPU)
#define MLN_USE_FILL_EXTRUSION_INSTANCING (MLN_RENDER_BACKEND_METAL || MLN_RENDER_BACKEND_VULKAN)
#define MLN_USE_SYMBOL_INSTANCING (MLN_RENDER_BACKEND_METAL || MLN_RENDER_BACKEND_VULKAN)
#define MLN_USE_FILL_ROUNDED_CORNER (MLN_RENDER_BACKEND_METAL || MLN_RENDER_BACKEND_VULKAN)

} // namespace shaders
} // namespace mln
