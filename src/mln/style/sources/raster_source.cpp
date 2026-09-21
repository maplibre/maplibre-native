#include <mln/storage/file_source.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/tileset.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/sources/raster_source.hpp>
#include <mln/style/sources/tile_source_impl.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/async_request.hpp>
#include <mln/util/exception.hpp>
#include <mln/util/mapbox.hpp>

namespace mln {
namespace style {

RasterSource::RasterSource(std::string id,
                           variant<std::string, Tileset> urlOrTileset_,
                           uint16_t tileSize,
                           SourceType sourceType)
    : TileSource(id, urlOrTileset_, tileSize, sourceType) {}

bool RasterSource::supportsLayerType(const mln::style::LayerTypeInfo* info) const {
    return mln::underlying_type(Tile::Kind::Raster) == mln::underlying_type(info->tileKind);
}

} // namespace style
} // namespace mln
