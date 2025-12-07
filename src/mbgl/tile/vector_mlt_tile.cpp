#include <mbgl/tile/vector_mlt_tile.hpp>

#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/tile/vector_mlt_tile_data.hpp>

#include <utility>

namespace mbgl {

VectorMLTTile::VectorMLTTile(const OverscaledTileID& id_,
                             std::string sourceID_,
                             const TileParameters& parameters_,
                             const Tileset& tileset_,
                             TileObserver* observer_)
    : VectorTile(id_, std::move(sourceID_), parameters_, tileset_, observer_) {}

VectorMLTTile::~VectorMLTTile() {
    // Don't rely on `~TileLoader` to close, it's not safe to call there.
    // We're still calling a virtual method from a destructor, so any overrides will not be called.
    GeometryTile::cancel();
}

void VectorMLTTile::setData(const std::shared_ptr<const std::string>& data_) {
    if (!obsolete) {
        GeometryTile::setData(data_ ? std::make_unique<VectorMLTTileData>(data_) : nullptr);
    }
}

} // namespace mbgl
