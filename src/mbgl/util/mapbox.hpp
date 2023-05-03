#pragma once

#include <string>
#include <mbgl/style/types.hpp>
#include <mbgl/util/tile_server_options.hpp>

namespace mbgl {

class Tileset;

namespace util {
namespace mapbox {

bool isCanonicalURL(const TileServerOptions& tileServerOptions, const std::string& url);

std::string normalizeSourceURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& apiKey);
std::string normalizeStyleURL(const TileServerOptions& tileServerOptions,
                              const std::string& str,
                              const std::string& apiKey);
std::string normalizeSpriteURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& apiKey);
std::string normalizeGlyphsURL(const TileServerOptions& tileServerOptions,
                               const std::string& str,
                               const std::string& apiKey);
std::string normalizeTileURL(const TileServerOptions& tileServerOptions,
                             const std::string& str,
                             const std::string& apiKey);

// Return a "mapbox://tiles/..." URL (suitable for normalizeTileURL) for the given Mapbox tile URL.
std::string canonicalizeTileURL(const TileServerOptions& tileServerOptions,
                                const std::string& str,
                                style::SourceType,
                                uint16_t tileSize);
std::string canonicalizeSourceURL(const TileServerOptions& tileServerOptions, const std::string& url);
std::string canonicalizeSpriteURL(const TileServerOptions& tileServerOptions, const std::string& url);
std::string canonicalizeGlyphURL(const TileServerOptions& tileServerOptions, const std::string& url);

// Replace URL templates with "mapbox://tiles/..." URLs (suitable for normalizeTileURL).
void canonicalizeTileset(
    const TileServerOptions& tileServerOptions, Tileset&, const std::string& url, style::SourceType, uint16_t tileSize);

extern const uint64_t DEFAULT_OFFLINE_TILE_COUNT_LIMIT;

} // namespace mapbox
} // namespace util
} // namespace mbgl
