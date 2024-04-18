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
    float pad1, pad2, pad3;
};
static_assert(sizeof(GlobalPaintParamsUBO) == 3 * 16);

enum {
    idGlobalPaintParamsUBO,
    globalUBOCount
};

#ifdef GLOBAL_STATE_UBO
// Background
const Size atlasSize = parameters.patternAtlas.getPixelSize();
/* .texsize = */ {static_cast<float>(atlasSize.width), static_cast<float>(atlasSize.height)},

    // Collision
    /*.camera_to_center_distance*/ parameters.state.getCameraToCenterDistance(),

    // Fill
    /*.units_to_pixels=*/{1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]},

    // Heatmap texture
    const auto& size = parameters.staticData.backendSize;
/* .matrix = */ util::cast<float>(viewportMat),
    /* .world = */ {static_cast<float>(size.width), static_cast<float>(size.height)},

    // Line
    /*units_to_pixels = */ {1.0f / parameters.pixelsToGLUnits[0], 1.0f / parameters.pixelsToGLUnits[1]}, 0, 0
};

// Symbol
const auto camDist = state.getCameraToCenterDistance();
/*.fade_change=*/parameters.symbolFadeChange,
    /*.camera_to_center_distance=*/camDist,
    /*.aspect_ratio=*/state.getSize().aspectRatio(),
#endif

} // namespace shaders
} // namespace mbgl
