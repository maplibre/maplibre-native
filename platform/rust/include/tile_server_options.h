#pragma once
#include "rust/cxx.h"
#include "mbgl/util/tile_server_options.hpp"

namespace mln {
namespace bridge {

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

// TileServerOptions setters
inline void TileServerOptions_withBaseURL(TileServerOptions& self, const rust::Str value) {
    self.withBaseURL((std::string)value);
}
inline void TileServerOptions_withUriSchemeAlias(TileServerOptions& self, const rust::Str value) {
    self.withUriSchemeAlias((std::string)value);
}
inline void TileServerOptions_withSourceTemplate(TileServerOptions& self,
                                                 const rust::Str source_template,
                                                 const rust::Str domain_name,
                                                 const int8_t* version_prefix) {
    self.withSourceTemplate((std::string)source_template,
                            (std::string)domain_name,
                            (version_prefix ? std::string((const char*)version_prefix) : std::optional<std::string>{}));
}
inline void TileServerOptions_withStyleTemplate(TileServerOptions& self,
                                                const rust::Str style_template,
                                                const rust::Str domain_name,
                                                const int8_t* version_prefix) {
    self.withStyleTemplate((std::string)style_template,
                           (std::string)domain_name,
                           (version_prefix ? std::string((const char*)version_prefix) : std::optional<std::string>{}));
}
inline void TileServerOptions_withSpritesTemplate(TileServerOptions& self,
                                                  const rust::Str sprites_template,
                                                  const rust::Str domain_name,
                                                  const int8_t* version_prefix) {
    self.withSpritesTemplate(
        (std::string)sprites_template,
        (std::string)domain_name,
        (version_prefix ? std::string((const char*)version_prefix) : std::optional<std::string>{}));
}
inline void TileServerOptions_withGlyphsTemplate(TileServerOptions& self,
                                                 const rust::Str glyphs_template,
                                                 const rust::Str domain_name,
                                                 const int8_t* version_prefix) {
    self.withGlyphsTemplate((std::string)glyphs_template,
                            (std::string)domain_name,
                            (version_prefix ? std::string((const char*)version_prefix) : std::optional<std::string>{}));
}
inline void TileServerOptions_withTileTemplate(TileServerOptions& self,
                                               const rust::Str tile_template,
                                               const rust::Str domain_name,
                                               const int8_t* version_prefix) {
    self.withTileTemplate((std::string)tile_template,
                          (std::string)domain_name,
                          (version_prefix ? std::string((const char*)version_prefix) : std::optional<std::string>{}));
}
inline void TileServerOptions_withApiKeyParameterName(TileServerOptions& self, const rust::Str value) {
    self.withApiKeyParameterName((std::string)value);
}
inline void TileServerOptions_setRequiresApiKey(TileServerOptions& self, bool value) {
    self.setRequiresApiKey(value);
}
inline void TileServerOptions_withDefaultStyle(TileServerOptions& self, const rust::Str value) {
    self.withDefaultStyle((std::string)value);
}

} // namespace bridge
} // namespace mln
