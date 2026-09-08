#pragma once

#include <mln/shaders/layer_ubo.hpp>

#include <array>

namespace mln {
namespace shaders {

/// Per-drawable data for the ground-shadow draw.
///
/// Consolidated into an SSBO array under MLN_UBO_CONSOLIDATION and indexed by idGlobalUBOIndex,
/// exactly like FillExtrusionDrawableUBO.
struct alignas(16) FillExtrusionShadowDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /// Tile units of horizontal shear per metre of vertex z. This already folds in the shadow
    /// length, the azimuth direction and the metres-to-tile-units conversion, which depends on
    /// the tile's zoom level and so cannot live in the layer-wide UBO below.
    /* 64 */ std::array<float, 2> offset_per_meter;

    // Interpolations
    /* 72 */ float base_t;
    /* 76 */ float height_t;
    /* 80 */
};
static_assert(sizeof(FillExtrusionShadowDrawableUBO) == 5 * 16);

/// Evaluated shadow properties that do not depend on the tile.
///
/// APPEND-ONLY: to add a field, consume the `pad1` slot below or append a whole new 16-byte block.
/// Never insert in the middle and never reorder -- the Metal mirror in
/// include/mln/shaders/mtl/fill_extrusion_shadow.hpp must stay byte-identical, and both it and
/// the static_assert have to be updated in the same commit.
struct alignas(16) FillExtrusionShadowPropsUBO {
    /*  0 */ Color color;
    /* 16 */ float opacity;

    // Uniform (non-data-driven) fill-extrusion geometry, used when the corresponding
    // HAS_UNIFORM_u_* permutation is compiled. These must match the values the fill-extrusion
    // shaders themselves use, or the shadow will not line up with the building.
    /* 20 */ float base;
    /* 24 */ float height;
    /* 28 */ float pad1;
    /* 32 */
};
static_assert(sizeof(FillExtrusionShadowPropsUBO) == 2 * 16);

} // namespace shaders
} // namespace mln
