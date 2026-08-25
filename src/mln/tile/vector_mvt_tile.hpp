#pragma once

#include <mln/tile/geometry_tile.hpp>
#include <mln/tile/tile_loader.hpp>
#include <mln/tile/vector_tile.hpp>

namespace mln {

class Tileset;
class TileParameters;

class VectorMVTTile final : public VectorTile {
public:
    VectorMVTTile(const OverscaledTileID&,
                  std::string sourceID,
                  const TileParameters&,
                  const Tileset&,
                  TileObserver* observer = nullptr);

    ~VectorMVTTile() override;

    void setData(const std::shared_ptr<const std::string>& data) override;
};

} // namespace mln
