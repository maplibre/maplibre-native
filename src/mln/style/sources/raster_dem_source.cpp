#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/tileset.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/sources/raster_dem_source.hpp>
#include <mln/style/sources/tile_source_impl.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/mapbox.hpp>

#include <utility>

namespace mln {
namespace style {

RasterDEMSource::RasterDEMSource(std::string id,
                                 variant<std::string, Tileset> urlOrTileset_,
                                 uint16_t tileSize,
                                 std::optional<SourceOptions> options_)
    : RasterSource(std::move(id), std::move(urlOrTileset_), tileSize, SourceType::RasterDEM),
      options(std::move(options_)) {}

RasterDEMSource::~RasterDEMSource() {
    // Invalidate weak pointers before RasterDEMSource members are destroyed
    invalidateWeakPtrsEarly();
}

bool RasterDEMSource::supportsLayerType(const mln::style::LayerTypeInfo* info) const {
    return mln::underlying_type(Tile::Kind::RasterDEM) == mln::underlying_type(info->tileKind);
}

void RasterDEMSource::setTilesetOverrides(Tileset& tileset) {
    if (options) {
        if (const auto encoding = options->rasterEncoding) {
            tileset.rasterEncoding = encoding;
        }
    }
}

} // namespace style
} // namespace mln
