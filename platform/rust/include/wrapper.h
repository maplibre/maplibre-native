#pragma once
#include "rust/cxx.h"
#include "mbgl/math/log2.hpp"
#include "mbgl/util/tile_server_options.hpp"

namespace ml {
namespace bridge {

uint32_t get_42();

using namespace mbgl;

// TileServerOptions constructor helpers
std::unique_ptr<TileServerOptions> TileServerOptions_new();
std::unique_ptr<TileServerOptions> TileServerOptions_default();
std::unique_ptr<TileServerOptions> TileServerOptions_mapbox();
std::unique_ptr<TileServerOptions> TileServerOptions_maplibre();
std::unique_ptr<TileServerOptions> TileServerOptions_maptiler();

// TileServerOptions Optional<string> helpers (not supported yet by cxx)
const int8_t* TileServerOptions_sourceVersionPrefix(const TileServerOptions& self);
const int8_t* TileServerOptions_styleVersionPrefix(const TileServerOptions& self);
const int8_t* TileServerOptions_spritesVersionPrefix(const TileServerOptions& self);
const int8_t* TileServerOptions_glyphsVersionPrefix(const TileServerOptions& self);
const int8_t* TileServerOptions_tileVersionPrefix(const TileServerOptions& self);

} // namespace bridge
} // namespace ml
