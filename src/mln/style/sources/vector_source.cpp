#include <mln/storage/file_source.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/tileset.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/sources/vector_source.hpp>
#include <mln/style/sources/tile_source_impl.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/async_request.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/exception.hpp>
#include <mln/util/mapbox.hpp>
#include <string>

namespace mln {
namespace style {

VectorSource::VectorSource(std::string id,
                           variant<std::string, Tileset> urlOrTileset_,
                           std::optional<float> maxZoom_,
                           std::optional<float> minZoom_,
                           Tileset::VectorEncoding encoding_)
    : TileSource(id, urlOrTileset_, util::tileSize_I, SourceType::Vector),
      maxZoom(std::move(maxZoom_)),
      minZoom(std::move(minZoom_)),
      encoding(encoding_) {}

void VectorSource::setTilesetOverrides(Tileset& tileset) {
    if (maxZoom) {
        tileset.zoomRange.max = static_cast<uint8_t>(*maxZoom);
    }
    if (minZoom) {
        tileset.zoomRange.min = static_cast<uint8_t>(*minZoom);
    }
}

const std::vector<std::string> VectorSource::getTiles() const {
    auto tileset = impl().tileset;
    if (tileset.has_value()) {
        return tileset->tiles;
    } else {
        return {};
    }
}

void VectorSource::setTiles(const std::vector<std::string>& tiles) {
    auto& tileset = impl().tileset;
    if (!tileset.has_value()) return;
    if (tileset->tiles == tiles) return;
    Tileset newtileset(*tileset);
    newtileset.tiles = tiles;
    baseImpl = makeMutable<Impl>(impl(), newtileset);
    observer->onSourceChanged(*this);
}

bool VectorSource::supportsLayerType(const mln::style::LayerTypeInfo* info) const {
    return mln::underlying_type(Tile::Kind::Geometry) == mln::underlying_type(info->tileKind);
}

} // namespace style
} // namespace mln
