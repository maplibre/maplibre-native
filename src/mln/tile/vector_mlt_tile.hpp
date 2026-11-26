#pragma once

#include <mln/tile/geometry_tile.hpp>
#include <mln/tile/tile_loader.hpp>
#include <mln/tile/vector_tile.hpp>

namespace mln {

class Tileset;
class TileParameters;

class VectorMLTTile final : public VectorTile {
public:
    VectorMLTTile(const OverscaledTileID&,
                  std::string sourceID,
                  const TileParameters&,
                  const Tileset&,
                  TileObserver* observer,
                  bool fastPFOREnabled);

    ~VectorMLTTile() override;

    void setData(const std::shared_ptr<const std::string>& data) override;

private:
    bool fastPFOREnabled;
};

} // namespace mln
