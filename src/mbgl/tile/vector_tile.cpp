#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/tile/vector_tile.hpp>
#include <mbgl/tile/vector_tile_data.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>
#include <utility>

namespace mbgl {

VectorTile::VectorTile(const OverscaledTileID& id_,
                       std::string sourceID_,
                       const TileParameters& parameters,
                       const Tileset& tileset,
                       TileObserver* observer_)
    : GeometryTile(id_, std::move(sourceID_), parameters, observer_),
      loader(*this, id_, parameters, tileset) {}

VectorTile::~VectorTile() {
    // Don't rely on `~TileLoader` to close, it's not safe to call there.
    // We're still calling a virtual method from a destructor, so any overrides will not be called.
    GeometryTile::cancel();
}

void VectorTile::setNecessity(TileNecessity necessity) {
    mbgl::Log::Info(mbgl::Event::General, "VectorTile::setNecessity called for tile " + util::toString(id) + 
                    " with necessity: " + std::to_string(static_cast<int>(necessity)));
    loader.setNecessity(necessity);
}

void VectorTile::setUpdateParameters(const TileUpdateParameters& params) {
    loader.setUpdateParameters(params);
}

void VectorTile::setMetadata(std::optional<Timestamp> modified_, std::optional<Timestamp> expires_) {
    modified = std::move(modified_);
    expires = std::move(expires_);
}

void VectorTile::setData(const std::shared_ptr<const std::string>& data_) {
    mbgl::Log::Info(mbgl::Event::General, "VectorTile::setData called for tile " + util::toString(id) + ", data size=" + (data_ ? std::to_string(data_->size()) : "null"));
    if (obsolete) {
        mbgl::Log::Info(mbgl::Event::General, "VectorTile::setData - tile is obsolete");
        return;
    }
    
    if (!data_) {
        mbgl::Log::Warning(mbgl::Event::General, "VectorTile::setData - no data provided for tile " + util::toString(id));
    }

    GeometryTile::setData(data_ ? std::make_unique<VectorTileData>(data_) : nullptr);
}

} // namespace mbgl
