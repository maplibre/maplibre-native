#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <mbgl/tile/tile_operation.hpp>
#include <mbgl/util/symbol_error_observer.hpp>

#include <exception>

namespace mbgl {

class Tile;

class TileObserver : public SymbolErrorObserver {
public:
    virtual ~TileObserver() = default;

    virtual void onTileChanged(Tile&) {}
    virtual void onTileError(Tile&, std::exception_ptr) {}
    virtual void onTileAction(OverscaledTileID, std::string, TileOperation) {}
};

} // namespace mbgl
