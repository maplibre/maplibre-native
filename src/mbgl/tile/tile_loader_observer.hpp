#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <mbgl/tile/tile_operation.hpp>

namespace mln {

class Tile;

class TileLoaderObserver {
public:
    virtual ~TileLoaderObserver() = default;

    virtual void onTileAction(TileOperation) {}
};

} // namespace mln
