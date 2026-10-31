#pragma once

#include <mln/shaders/layer_ubo.hpp>

namespace mln {
namespace shaders {

struct alignas(16) HeatmapDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ float extrude_scale;

    // Interpolations
    /* 68 */ float weight_t;
    /* 72 */ float radius_t;
    /* 76 */ float pad1;

    // 3D terrain elevation; see RenderTerrain::getTerrainData
    /*  80 */ std::array<float, 4> dem_coords; // scale, x offset, y offset into the DEM tile, unused
    /*  96 */ std::array<float, 4> dem_unpack; // DEM unpack vector for the source's encoding
    /* 112 */ float dem_dim;
    /* 116 */ float dem_exaggeration;
    /* 120 */ float dem_enabled;
    /* 124 */ float pad2;
    /* 128 */
};
static_assert(sizeof(HeatmapDrawableUBO) == 8 * 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) HeatmapEvaluatedPropsUBO {
    /*  0 */ float weight;
    /*  4 */ float radius;
    /*  8 */ float intensity;
    /* 12 */ float padding;
    /* 16 */
};
static_assert(sizeof(HeatmapEvaluatedPropsUBO) == 16);

} // namespace shaders
} // namespace mln
