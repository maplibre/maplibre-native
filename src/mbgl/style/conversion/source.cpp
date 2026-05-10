#include <mbgl/style/conversion/source.hpp>
#include <mbgl/style/conversion/coordinate.hpp>
#include <mbgl/style/conversion/geojson.hpp>
#include <mbgl/style/conversion/geojson_options.hpp>
#include <mbgl/style/conversion/source_options.hpp>
#include <mbgl/style/conversion/tileset.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/sources/contour_source.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/style/sources/raster_dem_source.hpp>
#include <mbgl/style/sources/vector_source.hpp>
#include <mbgl/style/sources/image_source.hpp>
#include <mbgl/util/geo.hpp>

#include <cmath>

namespace mbgl {
namespace style {
namespace conversion {

namespace {
// A tile source can either specify a URL to TileJSON, or inline TileJSON.
std::optional<variant<std::string, Tileset>> convertURLOrTileset(const Convertible& value, Error& error) {
    auto urlVal = objectMember(value, "url");
    if (!urlVal) {
        std::optional<Tileset> tileset = convert<Tileset>(value, error);
        if (!tileset) {
            return std::nullopt;
        }
        return {std::move(*tileset)};
    }

    std::optional<std::string> url = toString(*urlVal);
    if (!url) {
        error.message = "source url must be a string";
        return std::nullopt;
    }

    return {std::move(*url)};
}

std::optional<std::unique_ptr<Source>> convertRasterSource(const std::string& id,
                                                           const Convertible& value,
                                                           Error& error) {
    std::optional<variant<std::string, Tileset>> urlOrTileset = convertURLOrTileset(value, error);
    if (!urlOrTileset) {
        return std::nullopt;
    }

    uint16_t tileSize = util::tileSize_I;
    auto tileSizeValue = objectMember(value, "tileSize");
    if (tileSizeValue) {
        std::optional<float> size = toNumber(*tileSizeValue);
        if (!size || *size < 0 || *size > std::numeric_limits<uint16_t>::max()) {
            error.message = "invalid tileSize";
            return std::nullopt;
        }
        tileSize = static_cast<uint16_t>(*size);
    }

    return {std::make_unique<RasterSource>(id, std::move(*urlOrTileset), tileSize)};
}

std::optional<std::unique_ptr<Source>> convertRasterDEMSource(const std::string& id,
                                                              const Convertible& value,
                                                              Error& error) {
    std::optional<variant<std::string, Tileset>> urlOrTileset = convertURLOrTileset(value, error);
    if (!urlOrTileset) {
        return std::nullopt;
    }

    uint16_t tileSize = util::tileSize_I;
    auto tileSizeValue = objectMember(value, "tileSize");
    if (tileSizeValue) {
        std::optional<float> size = toNumber(*tileSizeValue);
        if (!size || *size < 0 || *size > std::numeric_limits<uint16_t>::max()) {
            error.message = "invalid tileSize";
            return std::nullopt;
        }
        tileSize = static_cast<uint16_t>(*size);
    }

    /*
     TODO: Refactor TileJSON parsing responsibilities. Currently `Tileset` handles full TileJSON parsing with optional
           parameters. Since style.json can contain TileJSON parameter overrides or non-TileJSON spec parameters, a
           separate mechanism is needed. Ideally these two pathways share parsing logic.
    */
    auto options = convert<SourceOptions>(value, error);

    return {std::make_unique<RasterDEMSource>(id, std::move(*urlOrTileset), tileSize, options)};
}

std::optional<std::unique_ptr<Source>> convertVectorSource(const std::string& id,
                                                           const Convertible& value,
                                                           Error& error) {
    auto urlOrTileset = convertURLOrTileset(value, error);
    if (!urlOrTileset) {
        return std::nullopt;
    }
    const auto maxzoomValue = objectMember(value, "maxzoom");
    std::optional<float> maxzoom;
    if (maxzoomValue) {
        maxzoom = toNumber(*maxzoomValue);
        if (!maxzoom || *maxzoom < 0 || *maxzoom > std::numeric_limits<uint8_t>::max()) {
            error.message = "invalid maxzoom";
            return std::nullopt;
        }
    }
    const auto minzoomValue = objectMember(value, "minzoom");
    std::optional<float> minzoom;
    if (minzoomValue) {
        minzoom = toNumber(*minzoomValue);
        if (!minzoom || *minzoom < 0 || *minzoom > std::numeric_limits<uint8_t>::max()) {
            error.message = "invalid minzoom";
            return std::nullopt;
        }
    }
    const auto encodingValue = objectMember(value, "encoding");
    auto encoding = Tileset::VectorEncoding::Mapbox;
    if (encodingValue) {
        const auto encodingStr = toString(*encodingValue);
        if (encodingStr && encodingStr == "mvt") {
            encoding = Tileset::VectorEncoding::Mapbox;
        } else if (encodingStr && encodingStr == "mlt") {
            encoding = Tileset::VectorEncoding::MLT;
        } else {
            error.message = "invalid encoding";
            return std::nullopt;
        }
    }
    return {std::make_unique<VectorSource>(id, std::move(*urlOrTileset), maxzoom, minzoom, encoding)};
}

std::optional<std::unique_ptr<Source>> convertGeoJSONSource(const std::string& id,
                                                            const Convertible& value,
                                                            Error& error) {
    auto dataValue = objectMember(value, "data");
    if (!dataValue) {
        error.message = "GeoJSON source must have a data value";
        return std::nullopt;
    }

    Immutable<GeoJSONOptions> options = GeoJSONOptions::defaultOptions();
    if (std::optional<GeoJSONOptions> converted = convert<GeoJSONOptions>(value, error)) {
        options = makeMutable<GeoJSONOptions>(std::move(*converted));
    }

    auto result = std::make_unique<GeoJSONSource>(id, std::move(options));

    if (isObject(*dataValue)) {
        auto geoJSON = convert<GeoJSON>(*dataValue, error);
        if (!geoJSON) {
            return std::nullopt;
        }
        result->setGeoJSON(*geoJSON);
    } else if (toString(*dataValue)) {
        result->setURL(*toString(*dataValue));
    } else {
        error.message = "GeoJSON data must be a URL or an object";
        return std::nullopt;
    }

    return {std::move(result)};
}

std::optional<std::unique_ptr<Source>> convertContourSource(const std::string& id,
                                                            const Convertible& value,
                                                            Error& error) {
    ContourSourceOptions options;

    // Required: source — ID of the upstream raster-dem source.
    auto sourceVal = objectMember(value, "source");
    if (!sourceVal) {
        error.message = "contour source must reference a `source`";
        return std::nullopt;
    }
    auto sourceStr = toString(*sourceVal);
    if (!sourceStr) {
        error.message = "contour `source` must be a string";
        return std::nullopt;
    }
    options.sourceID = std::move(*sourceStr);

    // Required: intervals — odd-length number array of step-by-zoom outputs
    // and stops, with strictly-positive outputs.
    auto intervalsVal = objectMember(value, "intervals");
    if (!intervalsVal) {
        error.message = "contour source must specify `intervals`";
        return std::nullopt;
    }
    if (!isArray(*intervalsVal) || arrayLength(*intervalsVal) == 0) {
        error.message = "contour `intervals` must be a non-empty array";
        return std::nullopt;
    }
    const std::size_t n = arrayLength(*intervalsVal);
    if ((n % 2) == 0) {
        error.message = "contour `intervals` must have an odd number of entries (output, stop, output, stop, ..., output)";
        return std::nullopt;
    }
    options.intervals.stops.reserve(n);
    for (std::size_t i = 0; i < n; i++) {
        auto entry = toDouble(arrayMember(*intervalsVal, i));
        if (!entry) {
            error.message = "contour `intervals` entries must be numbers";
            return std::nullopt;
        }
        options.intervals.stops.push_back(*entry);
    }
    if (!algorithm::contour::isValid(options.intervals)) {
        error.message = "contour `intervals` outputs must be > 0 and stops strictly increasing";
        return std::nullopt;
    }

    // Optional: unit — "meters" (default), "feet", or a positive number used
    // as a metres-to-display multiplier.
    if (auto unitVal = objectMember(value, "unit")) {
        if (auto unitStr = toString(*unitVal)) {
            if (*unitStr == "meters") {
                options.unit.unit = algorithm::contour::ContourUnit::Meters;
            } else if (*unitStr == "feet") {
                options.unit.unit = algorithm::contour::ContourUnit::Feet;
            } else {
                error.message = "contour `unit` must be \"meters\", \"feet\", or a positive number";
                return std::nullopt;
            }
        } else if (auto unitNum = toDouble(*unitVal)) {
            if (*unitNum <= 0.0) {
                error.message = "contour `unit` numeric multiplier must be > 0";
                return std::nullopt;
            }
            options.unit.unit = algorithm::contour::ContourUnit::Custom;
            options.unit.customMultiplier = *unitNum;
        } else {
            error.message = "contour `unit` must be \"meters\", \"feet\", or a positive number";
            return std::nullopt;
        }
    }

    // Optional: majorMultiplier — step-by-zoom array of positive integers
    // (same odd-length shape as `intervals`). A line at tile-zoom interval
    // `i` and resolved multiplier `m` emits `major: true` iff `e % (i*m)
    // == 0`. Defaults to a single-output schedule of `{5}` if absent.
    if (auto majorVal = objectMember(value, "majorMultiplier")) {
        if (!isArray(*majorVal) || arrayLength(*majorVal) == 0) {
            error.message = "contour `majorMultiplier` must be a non-empty array";
            return std::nullopt;
        }
        const std::size_t mn = arrayLength(*majorVal);
        if ((mn % 2) == 0) {
            error.message = "contour `majorMultiplier` must have an odd number of entries (output, stop, output, stop, ..., output)";
            return std::nullopt;
        }
        algorithm::contour::IntervalSchedule majorSchedule;
        majorSchedule.stops.reserve(mn);
        for (std::size_t i = 0; i < mn; i++) {
            auto entry = toDouble(arrayMember(*majorVal, i));
            if (!entry) {
                error.message = "contour `majorMultiplier` entries must be numbers";
                return std::nullopt;
            }
            // Output positions (even indices) must be positive integers.
            if ((i % 2) == 0) {
                if (*entry <= 0.0 || *entry != std::floor(*entry)) {
                    error.message = "contour `majorMultiplier` outputs must be positive integers";
                    return std::nullopt;
                }
            }
            majorSchedule.stops.push_back(*entry);
        }
        if (!algorithm::contour::isValid(majorSchedule)) {
            error.message = "contour `majorMultiplier` outputs must be > 0 and stops strictly increasing";
            return std::nullopt;
        }
        options.majorMultiplier = std::move(majorSchedule);
    }

    // Optional: overzoom — non-negative integer count of bilinear-upsample
    // levels above the upstream DEM source. Capped at 4 — beyond that
    // the bilinear upsample produces 16+× the source data per tile and
    // visual quality stops improving.
    if (auto overzoomVal = objectMember(value, "overzoom")) {
        auto overzoomNum = toDouble(*overzoomVal);
        if (!overzoomNum || *overzoomNum < 0.0 || *overzoomNum > 4.0 ||
            *overzoomNum != std::floor(*overzoomNum)) {
            error.message = "contour `overzoom` must be an integer in [0, 4]";
            return std::nullopt;
        }
        options.overzoom = static_cast<std::uint8_t>(*overzoomNum);
    }

    return {std::make_unique<ContourSource>(id, std::move(options))};
}

std::optional<std::unique_ptr<Source>> convertImageSource(const std::string& id,
                                                          const Convertible& value,
                                                          Error& error) {
    auto urlValue = objectMember(value, "url");
    if (!urlValue) {
        error.message = "Image source must have a url value";
        return std::nullopt;
    }

    auto urlString = toString(*urlValue);
    if (!urlString) {
        error.message = "Image url must be a URL string";
        return std::nullopt;
    }

    auto coordinatesValue = objectMember(value, "coordinates");
    if (!coordinatesValue) {
        error.message = "Image source must have a coordinates values";
        return std::nullopt;
    }

    if (!isArray(*coordinatesValue) || arrayLength(*coordinatesValue) != 4) {
        error.message =
            "Image coordinates must be an array of four longitude latitude "
            "pairs";
        return std::nullopt;
    }

    std::array<LatLng, 4> coordinates;
    for (std::size_t i = 0; i < 4; i++) {
        auto latLng = conversion::convert<LatLng>(arrayMember(*coordinatesValue, i), error);
        if (!latLng) {
            return std::nullopt;
        }
        coordinates[i] = *latLng;
    }
    auto result = std::make_unique<ImageSource>(id, coordinates);
    result->setURL(*urlString);

    return {std::move(result)};
}
} // namespace

std::optional<std::unique_ptr<Source>> Converter<std::unique_ptr<Source>>::operator()(const Convertible& value,
                                                                                      Error& error,
                                                                                      const std::string& id) const {
    if (!isObject(value)) {
        error.message = "source must be an object";
        return std::nullopt;
    }

    auto typeValue = objectMember(value, "type");
    if (!typeValue) {
        error.message = "source must have a type";
        return std::nullopt;
    }

    std::optional<std::string> type = toString(*typeValue);
    if (!type) {
        error.message = "source type must be a string";
        return std::nullopt;
    }
    const std::string& tname = type.value();
    if (tname == "raster") {
        return convertRasterSource(id, value, error);
    } else if (tname == "raster-dem") {
        return convertRasterDEMSource(id, value, error);
    } else if (tname == "vector") {
        return convertVectorSource(id, value, error);
    } else if (tname == "geojson") {
        return convertGeoJSONSource(id, value, error);
    } else if (tname == "image") {
        return convertImageSource(id, value, error);
    } else if (tname == "contour") {
        return convertContourSource(id, value, error);
    } else {
        error.message = "invalid source type";
        return std::nullopt;
    }
}

} // namespace conversion
} // namespace style
} // namespace mbgl
