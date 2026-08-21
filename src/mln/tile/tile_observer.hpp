#pragma once

#include <mln/tile/tile_id.hpp>
#include <mln/tile/tile_operation.hpp>
#include <mln/util/symbol_error_observer.hpp>

#include <exception>

namespace mln {

class Tile;

class TileObserver : public SymbolErrorObserver {
public:
    ~TileObserver() override = default;

    virtual void onTileChanged(Tile&) {}
    virtual void onTileError(Tile&, std::exception_ptr) {}
    virtual void onTileAction(OverscaledTileID, std::string, TileOperation) {}
};

} // namespace mln
