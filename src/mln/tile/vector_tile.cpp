#include <mln/tile/vector_mvt_tile.hpp>

#include <mln/renderer/tile_parameters.hpp>
#include <mln/tile/tile_loader_impl.hpp>
#include <mln/util/accept_header.hpp>

#include <utility>

namespace mln {

namespace {

Tileset::VectorEncoding encodingOf(const Tileset& tileset) {
    return tileset.vectorEncoding.value_or(Tileset::VectorEncoding::Mapbox);
}

} // namespace

VectorTile::VectorTile(const OverscaledTileID& id_,
                       std::string sourceID_,
                       const TileParameters& parameters_,
                       const Tileset& tileset,
                       TileObserver* observer_)
    : GeometryTile(id_, std::move(sourceID_), parameters_, observer_),
      loader(std::make_unique<TileLoader<VectorTile>>(
          *this,
          id_,
          parameters_,
          tileset,
          encodingOf(tileset) == Tileset::VectorEncoding::MLT ? http::MIME_TYPE_MLT : http::MIME_TYPE_MVT,
          encodingOf(tileset))) {}

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

} // namespace mln
