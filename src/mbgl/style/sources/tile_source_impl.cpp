#include <mbgl/style/sources/tile_source_impl.hpp>

namespace mbgl {
namespace style {

TileSource::Impl::Impl(SourceType sourceType, std::string id_, uint16_t tileSize_ = util::tileSize_I)
    : Source::Impl(sourceType, std::move(id_)),
      tileSize(tileSize_) {}

TileSource::Impl::Impl(const Impl& other, Tileset tileset_)
    : Source::Impl(other),
      tileset(std::move(tileset_)),
      tileSize(other.tileSize) {}

uint16_t TileSource::Impl::getTileSize() const {
    return tileSize;
}

std::optional<std::string> TileSource::Impl::getAttribution() const {
    if (!tileset) {
        return {};
    }
    return tileset->attribution;
}

} // namespace style
} // namespace mbgl
