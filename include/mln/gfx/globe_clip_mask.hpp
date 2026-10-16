#pragma once

#include <mln/shaders/layer_ubo.hpp>
#include <mln/tile/tile_id.hpp>

#include <cstdint>

namespace mln {
namespace gfx {

/// A tile clipping mask on the globe: the tile's projection block, drawn over its pole-capped mesh.
struct GlobeClipMask {
    shaders::ProjectionUBO projection;
    uint32_t stencilRef;
    CanonicalTileID tile;
};

} // namespace gfx
} // namespace mln
