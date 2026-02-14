#pragma once
#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

// Maximum number of illumination sources supported
constexpr int MAX_ILLUMINATION_SOURCES = 4;

enum class HillshadeMethod : int32_t {
    Standard = 0,
    Combined = 1,
    Igor = 2,
    Multidirectional = 3,
    Basic = 4
};

struct alignas(16) HillshadeDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */
};
static_assert(sizeof(HillshadeDrawableUBO) == 4 * 16);

struct alignas(16) HillshadeTilePropsUBO {
    /*  0 */ std::array<float, 2> latrange;
    /*  8 */ float exaggeration; // NEW: replaces light[0] (intensity)
    /* 12 */ int32_t method;     // NEW: hillshade method (0-4)
    /* 16 */ int32_t num_lights; // NEW: number of light sources (1-4)
    /* 20 */ float pad0;         // Padding for alignment
    /* 24 */ float pad1;         // Padding for alignment
    /* 28 */ float pad2;         // Padding for alignment
    /* 32 */
};
static_assert(sizeof(HillshadeTilePropsUBO) == 32);

/// Evaluated properties that do not depend on the tile
struct alignas(16) HillshadeEvaluatedPropsUBO {
    /*   0 */ Color accent;
    /*  16 */ std::array<float, 4> altitudes;   // NEW: altitude values in radians (up to 4)
    /*  32 */ std::array<float, 4> azimuths;    // NEW: azimuth values in radians (up to 4)
    /*  48 */ std::array<float, 16> shadows;    // NEW: 4 shadow colors (4 colors × 4 RGBA)
    /* 112 */ std::array<float, 16> highlights; // NEW: 4 highlight colors (4 colors × 4 RGBA)
    /* 176 */
};
static_assert(sizeof(HillshadeEvaluatedPropsUBO) == 176);

} // namespace shaders
} // namespace mbgl
