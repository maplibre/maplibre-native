#include <mln/style/conversion/source_options.hpp>

#include <limits>
#include <string>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/expression/dsl.hpp>

namespace mln {
namespace style {
namespace conversion {

namespace {

std::optional<uint8_t> zoomMember(const Convertible& value, const char* name, Error& error, bool& invalid) {
    const auto member = objectMember(value, name);
    if (!member) {
        return std::nullopt;
    }
    const auto number = toNumber(*member);
    if (!number || *number < 0 || *number > std::numeric_limits<uint8_t>::max()) {
        error.message = std::string("invalid ") + name;
        invalid = true;
        return std::nullopt;
    }
    return static_cast<uint8_t>(*number);
}

} // namespace

std::optional<SourceOptions> Converter<SourceOptions>::operator()(const Convertible& value, Error& error) const {
    SourceOptions options;
    bool any = false;

    const auto encodingValue = objectMember(value, "encoding");
    if (encodingValue) {
        const auto encoding = toString(*encodingValue);
        if (encoding && *encoding == "terrarium") {
            options.rasterEncoding = Tileset::RasterEncoding::Terrarium;
        } else if (encoding && *encoding == "mapbox") {
            options.rasterEncoding = Tileset::RasterEncoding::Mapbox;
        } else if (encoding && *encoding == "mvt") {
            options.vectorEncoding = Tileset::VectorEncoding::Mapbox;
        } else if (encoding && *encoding == "mlt") {
            options.vectorEncoding = Tileset::VectorEncoding::MLT;
        } else {
            error.message =
                "invalid encoding - valid types are 'mapbox' and 'terrarium' for raster sources, 'mvt' and 'mlt' for "
                "vector sources";
            return std::nullopt;
        }
        any = true;
    }

    bool invalid = false;
    options.minzoom = zoomMember(value, "minzoom", error, invalid);
    if (invalid) {
        return std::nullopt;
    }
    options.maxzoom = zoomMember(value, "maxzoom", error, invalid);
    if (invalid) {
        return std::nullopt;
    }
    any = any || options.minzoom || options.maxzoom;

    if (!any) {
        return {};
    }
    return options;
}

} // namespace conversion
} // namespace style
} // namespace mln
