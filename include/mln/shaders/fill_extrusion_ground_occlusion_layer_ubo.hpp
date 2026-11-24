#pragma once

#include <mln/shaders/layer_ubo.hpp>

#include <array>

namespace mln {
namespace shaders {

/// Per-drawable data for the ground ambient occlusion around fill-extrusions.
///
/// Consolidated into an SSBO array under MLN_UBO_CONSOLIDATION and indexed by idGlobalUBOIndex,
/// exactly like FillExtrusionDrawableUBO. The Metal mirror in
/// include/mln/shaders/mtl/fill_extrusion_ground_occlusion.hpp must stay byte-identical.
struct alignas(16) FillExtrusionGroundOcclusionDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;

    // Interpolations
    /* 64 */ float base_t;
    /* 68 */ float height_t;

    /// Tile units per metre on the ground for this tile
    /* 72 */ float meters_to_tile_units;
    /// fill-extrusion-ambient-occlusion-ground-radius in metres
    /* 76 */ float radius;
    /* 80 */
};
static_assert(sizeof(FillExtrusionGroundOcclusionDrawableUBO) == 5 * 16);

/// Evaluated ground ambient occlusion properties that do not depend on the tile.
struct alignas(16) FillExtrusionGroundOcclusionPropsUBO {
    /// fill-extrusion-ambient-occlusion-ground-intensity, 0 when off
    /*  0 */ float intensity;

    // Uniform (non-data-driven) fill-extrusion geometry, used when the corresponding
    // HAS_UNIFORM_u_* permutation is compiled. These must match the values the fill-extrusion
    // shaders themselves use, or the occlusion will not line up with the building.
    /*  4 */ float base;
    /*  8 */ float height;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(FillExtrusionGroundOcclusionPropsUBO) == 1 * 16);

} // namespace shaders
} // namespace mln
