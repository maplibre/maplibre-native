#pragma once

#include <mbgl/util/range.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/geo.hpp>

#include <tuple>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>

namespace mbgl {

class Tileset {
public:
    enum class Scheme : bool {
        XYZ,
        TMS
    };
    enum class Encoding : std::uint8_t {
        Mapbox,    // Mapbox DEM for raster sources, MVT for vector sources
        Terrarium, // Terrarium DEM, only valid for raster sources
        MLT,       // MapLibre Tiles, only valid for vector sources
    };

    std::vector<std::string> tiles;
    Range<uint8_t> zoomRange;
    std::string attribution;
    Scheme scheme;
    // Encoding is not supported by the TileJSON spec
    Encoding encoding;
    std::optional<LatLngBounds> bounds;

    Tileset(std::vector<std::string> tiles_ = std::vector<std::string>(),
            Range<uint8_t> zoomRange_ = {0, util::DEFAULT_MAX_ZOOM},
            std::string attribution_ = {},
            Scheme scheme_ = Scheme::XYZ,
            Encoding encoding_ = Encoding::Mapbox)
        : tiles(std::move(tiles_)),
          zoomRange(zoomRange_),
          attribution(std::move(attribution_)),
          scheme(scheme_),
          encoding(encoding_) {};

    // TileJSON also includes center and zoom but they are not used by mbgl.

    bool operator==(const Tileset&) const = default;
    bool operator!=(const Tileset&) const = default;
};

} // namespace mbgl
