#pragma once

#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile_loader.hpp>
#include <mbgl/tile/vector_tile.hpp>

namespace mbgl {

class Tileset;
class TileParameters;

class VectorMVTTile : public VectorTile {
public:
    VectorMVTTile(const OverscaledTileID&,
                  std::string sourceID,
                  const TileParameters&,
                  const Tileset&,
                  TileObserver* observer = nullptr);

    ~VectorMVTTile() override;

    void setData(const std::shared_ptr<const std::string>& data) override;
};

} // namespace mbgl
