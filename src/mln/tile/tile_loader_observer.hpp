#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/tile/tile_operation.hpp>

namespace mln {

class Tile;

class TileLoaderObserver {
public:
    virtual ~TileLoaderObserver() = default;

    virtual void onTileAction(TileOperation) {}
};

} // namespace mln
