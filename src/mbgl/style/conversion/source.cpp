#include <mbgl/style/conversion/source.hpp>
#include <mbgl/style/conversion/coordinate.hpp>
#include <mbgl/style/conversion/geojson.hpp>
#include <mbgl/style/conversion/geojson_options.hpp>
#include <mbgl/style/conversion/source_options.hpp>
#include <mbgl/style/conversion/tileset.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/style/sources/raster_dem_source.hpp>
#include <mbgl/style/sources/vector_source.hpp>
#include <mbgl/style/sources/image_source.hpp>
#include <mbgl/util/geo.hpp>

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
    } else {
        error.message = "invalid source type";
        return std::nullopt;
    }
}

} // namespace conversion
} // namespace style
} // namespace mbgl
