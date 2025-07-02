#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/tileset.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/style/sources/raster_dem_source.hpp>
#include <mbgl/style/sources/tile_source_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/mapbox.hpp>
#include <utility>

namespace mbgl {
namespace style {

RasterDEMSource::RasterDEMSource(std::string id,
                                 variant<std::string, Tileset> urlOrTileset_,
                                 uint16_t tileSize,
                                 std::optional<RasterDEMOptions> options_)
    : RasterSource(std::move(id), std::move(urlOrTileset_), tileSize, SourceType::RasterDEM),
      options(std::move(options_)) {}

bool RasterDEMSource::supportsLayerType(const mbgl::style::LayerTypeInfo* info) const {
    return mbgl::underlying_type(Tile::Kind::RasterDEM) == mbgl::underlying_type(info->tileKind);
}

void RasterDEMSource::setTilesetOverrides(Tileset& tileset) {
    if (options) {
        if (std::optional<Tileset::DEMEncoding> encoding = options.value().encoding) {
            tileset.encoding = encoding.value();
        }
    }
}

} // namespace style
} // namespace mbgl
