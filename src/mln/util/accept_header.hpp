#pragma once

#include <mln/util/tileset.hpp>

#include <string_view>

namespace mln {
namespace http {

/// Media type of MapLibre Tiles (MLT), used for `Accept` headers and for MBTiles `format` metadata
inline constexpr std::string_view mltMediaType = "application/vnd.maplibre-tile";

/// The encoding a tileset asks for, defaulting to MVT
inline Tileset::VectorEncoding vectorEncodingOf(const Tileset& tileset) {
    return tileset.vectorEncoding.value_or(Tileset::VectorEncoding::Mapbox);
}

/// Only MLT is content negotiated; every other request is sent without an `Accept` header.
constexpr std::string_view vectorAcceptHeader(Tileset::VectorEncoding encoding) {
    return encoding == Tileset::VectorEncoding::MLT ? mltMediaType : std::string_view{};
}

} // namespace http
} // namespace mln
