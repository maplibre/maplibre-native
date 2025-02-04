#pragma once
#include "rust/cxx.h"
#include "mbgl/math/log2.hpp"
#include "mbgl/util/tile_server_options.hpp"

namespace ml {
namespace bridge {

uint32_t get_42();

using namespace mbgl;

// TileServerOptions constructor helpers
inline std::unique_ptr<TileServerOptions> TileServerOptions_new() {
    return std::make_unique<TileServerOptions>();
}
inline std::unique_ptr<TileServerOptions> TileServerOptions_mapbox() {
    return std::make_unique<TileServerOptions>(TileServerOptions::MapLibreConfiguration());
}
inline std::unique_ptr<TileServerOptions> TileServerOptions_maplibre() {
    return std::make_unique<TileServerOptions>(TileServerOptions::MapboxConfiguration());
}
inline std::unique_ptr<TileServerOptions> TileServerOptions_maptiler() {
    return std::make_unique<TileServerOptions>(TileServerOptions::MapTilerConfiguration());
}

// TileServerOptions getters for optional string fields
inline const int8_t* get_raw_str_ptr(const std::optional<std::string>& v) {
    return (const int8_t*)(v.has_value() ? v->c_str() : nullptr);
}
inline const int8_t* TileServerOptions_sourceVersionPrefix(const TileServerOptions& self) {
    return get_raw_str_ptr(self.sourceVersionPrefix());
}
inline const int8_t* TileServerOptions_styleVersionPrefix(const TileServerOptions& self) {
    return get_raw_str_ptr(self.styleVersionPrefix());
}
inline const int8_t* TileServerOptions_spritesVersionPrefix(const TileServerOptions& self) {
    return get_raw_str_ptr(self.spritesVersionPrefix());
}
inline const int8_t* TileServerOptions_glyphsVersionPrefix(const TileServerOptions& self) {
    return get_raw_str_ptr(self.glyphsVersionPrefix());
}
inline const int8_t* TileServerOptions_tileVersionPrefix(const TileServerOptions& self) {
    return get_raw_str_ptr(self.tileVersionPrefix());
}

} // namespace bridge
} // namespace ml
