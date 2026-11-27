#pragma once

#include <mln/util/range.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/geo.hpp>

#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include <optional>

namespace mln {

class Tileset {
public:
    enum class Scheme : bool {
        XYZ,
        TMS
    };
    enum class RasterEncoding : std::uint8_t {
        Mapbox,    // Mapbox DEM
        Terrarium, // Terrarium DEM
        Custom,    // Decoded with `rasterCustomFactors`
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
    /// The custom encoding's `redFactor`, `greenFactor`, `blueFactor` and `baseShift`: a pixel's elevation is
    /// red * redFactor + green * greenFactor + blue * blueFactor - baseShift, each channel from 0 to 255.
    std::array<float, 4> rasterCustomFactors{1, 1, 1, 0};
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

} // namespace mln
