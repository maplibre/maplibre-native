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
    enum class RasterEncoding : std::uint8_t {
        Mapbox,    // Mapbox DEM
        Terrarium, // Terrarium DEM
    };
    enum class VectorEncoding : std::uint8_t {
        Mapbox, // Mapbox Vector Tiles (MVT)
        MLT,    // MapLibre Tiles
    };

    std::vector<std::string> tiles;
    Range<uint8_t> zoomRange;
    std::string attribution;
    Scheme scheme;
    std::optional<RasterEncoding> rasterEncoding;
    std::optional<VectorEncoding> vectorEncoding;
    std::optional<LatLngBounds> bounds;

    Tileset(std::vector<std::string> tiles_ = std::vector<std::string>(),
            Range<uint8_t> zoomRange_ = {0, util::DEFAULT_MAX_ZOOM},
            std::string attribution_ = {},
            Scheme scheme_ = Scheme::XYZ,
            std::optional<RasterEncoding> rasterEncoding_ = std::nullopt,
            std::optional<VectorEncoding> encoding_ = std::nullopt)
        : tiles(std::move(tiles_)),
          zoomRange(zoomRange_),
          attribution(std::move(attribution_)),
          scheme(scheme_),
          rasterEncoding(rasterEncoding_),
          vectorEncoding(encoding_) {}

    // TileJSON also includes center and zoom but they are not used by mbgl.

    bool operator==(const Tileset&) const = default;
    bool operator!=(const Tileset&) const = default;
};

} // namespace mbgl
