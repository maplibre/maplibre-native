#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

namespace mbgl {
namespace shaders {

struct alignas(16) TerrainDrawableUBO {
    /*  0 */ std::array<float, 4 * 4> matrix;
    /* 64 */ std::array<float, 4> dem_coords; // scale, x offset, y offset into the bound DEM
                                              // tile ({1,0,0,0} unless an ancestor is bound)
    /* 80 */
};
static_assert(sizeof(TerrainDrawableUBO) == 5 * 16);

struct alignas(16) TerrainTilePropsUBO {
    /*  0 */ std::array<float, 2> dem_tl;
    /*  8 */ float dem_scale;
    /* 12 */ float pad1;
    /* 16 */
};
static_assert(sizeof(TerrainTilePropsUBO) == 16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) TerrainEvaluatedPropsUBO {
    /*  0 */ std::array<float, 4> unpack; // DEM unpack vector for the source's encoding
    /* 16 */ float exaggeration;
    /* 20 */ float elevation_offset;
    /* 24 */ float pad1;
    /* 28 */ float pad2;
    /* 32 */
};
static_assert(sizeof(TerrainEvaluatedPropsUBO) == 32);

} // namespace shaders
} // namespace mbgl
