#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <mbgl/tile/tile_operation.hpp>
#include <exception>

namespace mbgl {

class Tile;

class TileObserver {
public:
    virtual ~TileObserver() = default;

    virtual void onTileChanged(Tile&) {}
    virtual void onTileError(Tile&, std::exception_ptr) {}
    virtual void onTileAction(OverscaledTileID, std::string, TileOperation) {}
};

} // namespace mbgl
