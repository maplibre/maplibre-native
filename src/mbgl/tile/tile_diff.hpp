#pragma once

#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/util/immutable.hpp>
#include <mbgl/util/variant.hpp>

namespace mbgl {

class RenderTile;
using RenderTiles = std::shared_ptr<const std::vector<std::reference_wrapper<const RenderTile>>>;

struct TileDifference {
    std::vector<RenderTiles::element_type::value_type> added;
    std::vector<OverscaledTileID> removed;
    std::vector<RenderTiles::element_type::value_type> remainder;
};

/// @brief Compute the differences in tile IDs between two containers of `RenderTile`
TileDifference diffTiles(const RenderTiles&, const RenderTiles&);

} // namespace mbgl
