#include <mbgl/tile/vector_mvt_tile.hpp>

#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>

#include <utility>

namespace mbgl {

VectorTile::VectorTile(const OverscaledTileID& id_,
                       std::string sourceID_,
                       const TileParameters& parameters_,
                       const Tileset& tileset,
                       TileObserver* observer_)
    : GeometryTile(id_, std::move(sourceID_), parameters_, observer_),
      loader(std::make_unique<TileLoader<VectorTile>>(*this, id_, parameters_, tileset)) {}

VectorTile::~VectorTile() {}

void VectorTile::setNecessity(TileNecessity necessity) {
    loader->setNecessity(necessity);
}

void VectorTile::setUpdateParameters(const TileUpdateParameters& params) {
    loader->setUpdateParameters(params);
}

void VectorTile::setMetadata(std::optional<Timestamp> modified_, std::optional<Timestamp> expires_) {
    modified = std::move(modified_);
    expires = std::move(expires_);
}

} // namespace mbgl
