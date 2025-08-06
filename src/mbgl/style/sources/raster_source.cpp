#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/tileset.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/style/sources/tile_source_impl.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/mapbox.hpp>

namespace mbgl {
namespace style {

RasterSource::RasterSource(std::string id,
                           variant<std::string, Tileset> urlOrTileset_,
                           uint16_t tileSize,
                           SourceType sourceType)
    : TileSource(id, urlOrTileset_, tileSize, sourceType) {}

bool RasterSource::supportsLayerType(const mbgl::style::LayerTypeInfo* info) const {
    return mbgl::underlying_type(Tile::Kind::Raster) == mbgl::underlying_type(info->tileKind);
}

} // namespace style
} // namespace mbgl
