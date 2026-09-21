#pragma once

#include <mln/tile/geometry_tile.hpp>
#include <mln/tile/tile_loader.hpp>

namespace mln {

class Tileset;
class TileParameters;

class VectorTile : public GeometryTile {
public:
    VectorTile(const OverscaledTileID&,
               std::string sourceID,
               const TileParameters&,
               const Tileset&,
               TileObserver* observer = nullptr);
    ~VectorTile() override;

    void setNecessity(TileNecessity) final;
    void setUpdateParameters(const TileUpdateParameters&) final;
    void setMetadata(std::optional<Timestamp> modified, std::optional<Timestamp> expires);

    virtual void setData(const std::shared_ptr<const std::string>&) = 0;

protected:
    // this needs to be explicitly deleted in the most-derived destructor
    // see `~VectorMVTTile`
    std::unique_ptr<TileLoader<VectorTile>> loader;
};

} // namespace mln
