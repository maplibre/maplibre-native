#include "maplibre-native/include/wrapper.h"
#include "maplibre-native/src/lib.rs.h"

namespace ml {
namespace bridge {

uint32_t get_42() {
    return 42;
}

std::unique_ptr<TileServerOptions> TileServerOptions_new() {
  return std::make_unique<TileServerOptions>();
}
std::unique_ptr<TileServerOptions> TileServerOptions_mapbox() {
  return std::make_unique<TileServerOptions>(TileServerOptions::MapLibreConfiguration());
}
std::unique_ptr<TileServerOptions> TileServerOptions_maplibre() {
  return std::make_unique<TileServerOptions>(TileServerOptions::MapboxConfiguration());
}
std::unique_ptr<TileServerOptions> TileServerOptions_maptiler() {
  return std::make_unique<TileServerOptions>(TileServerOptions::MapTilerConfiguration());
}

// FIXME: there should be a helper function to convert std::optional<std::string> to const int8_t *

const int8_t * TileServerOptions_sourceVersionPrefix(const TileServerOptions& self) {
    auto const &v = self.sourceVersionPrefix();
    return (const int8_t *)(v.has_value() ? v->c_str() : nullptr);
}
const int8_t * TileServerOptions_styleVersionPrefix(const TileServerOptions& self) {
    auto const &v = self.styleVersionPrefix();
    return (const int8_t *)(v.has_value() ? v->c_str() : nullptr);
}
const int8_t * TileServerOptions_spritesVersionPrefix(const TileServerOptions& self) {
    auto const &v = self.spritesVersionPrefix();
    return (const int8_t *)(v.has_value() ? v->c_str() : nullptr);
}
const int8_t * TileServerOptions_glyphsVersionPrefix(const TileServerOptions& self) {
    auto const &v = self.glyphsVersionPrefix();
    return (const int8_t *)(v.has_value() ? v->c_str() : nullptr);
}
const int8_t * TileServerOptions_tileVersionPrefix(const TileServerOptions& self) {
    auto const &v = self.tileVersionPrefix();
    return (const int8_t *)(v.has_value() ? v->c_str() : nullptr);
}

} // namespace bridge
} // namespace ml
