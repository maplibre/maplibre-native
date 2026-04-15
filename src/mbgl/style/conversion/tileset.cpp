#include <mbgl/style/conversion/tileset.hpp>
#include <mbgl/style/conversion/source_options.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/math/clamp.hpp>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<Tileset> Converter<Tileset>::operator()(const Convertible& value, Error& error) const {
    Tileset result;

    auto tiles = objectMember(value, "tiles");
    if (!tiles) {
        error.message = "source must have tiles";
        return std::nullopt;
    }

    if (!isArray(*tiles)) {
        error.message = "source tiles must be an array";
        return std::nullopt;
    }

    for (std::size_t i = 0; i < arrayLength(*tiles); i++) {
        std::optional<std::string> urlTemplate = toString(arrayMember(*tiles, i));
        if (!urlTemplate) {
            error.message = "source tiles member must be a string";
            return std::nullopt;
        }
        result.tiles.push_back(std::move(*urlTemplate));
    }

    auto schemeValue = objectMember(value, "scheme");
    if (schemeValue) {
        std::optional<std::string> scheme = toString(*schemeValue);
        if (scheme && *scheme == "tms") {
            result.scheme = Tileset::Scheme::TMS;
        }
    }

    auto options = convert<SourceOptions>(value, error);
    if (options) {
        if (const auto encoding = options->rasterEncoding) {
            result.rasterEncoding = encoding;
        }
        if (const auto encoding = options->vectorEncoding) {
            result.vectorEncoding = encoding;
        }
    }

    auto minzoomValue = objectMember(value, "minzoom");
    if (minzoomValue) {
        std::optional<float> minzoom = toNumber(*minzoomValue);
        if (!minzoom || *minzoom < 0 || *minzoom > std::numeric_limits<uint8_t>::max()) {
            error.message = "invalid minzoom";
            return std::nullopt;
        }
        result.zoomRange.min = static_cast<uint8_t>(*minzoom);
    }

    auto maxzoomValue = objectMember(value, "maxzoom");
    if (maxzoomValue) {
        std::optional<float> maxzoom = toNumber(*maxzoomValue);
        if (!maxzoom || *maxzoom < 0 || *maxzoom > std::numeric_limits<uint8_t>::max()) {
            error.message = "invalid maxzoom";
            return std::nullopt;
        }
        result.zoomRange.max = static_cast<uint8_t>(*maxzoom);
    }

    auto attributionValue = objectMember(value, "attribution");
    if (attributionValue) {
        std::optional<std::string> attribution = toString(*attributionValue);
        if (!attribution) {
            error.message = "source attribution must be a string";
            return std::nullopt;
        }
        result.attribution = std::move(*attribution);
    }

    auto boundsValue = objectMember(value, "bounds");
    if (boundsValue) {
        if (!isArray(*boundsValue) || arrayLength(*boundsValue) != 4) {
            error.message =
                "bounds must be an array with left, bottom, top, and right "
                "values";
            return std::nullopt;
        }
        std::optional<double> left = toDouble(arrayMember(*boundsValue, 0));
        std::optional<double> bottom = toDouble(arrayMember(*boundsValue, 1));
        std::optional<double> right = toDouble(arrayMember(*boundsValue, 2));
        std::optional<double> top = toDouble(arrayMember(*boundsValue, 3));

        if (!left || !right || !bottom || !top) {
            error.message =
                "bounds array must contain numeric longitude and latitude "
                "values";
            return std::nullopt;
        }

        bottom = util::clamp(*bottom, -90.0, 90.0);
        top = util::clamp(*top, -90.0, 90.0);
        if (*top < *bottom) {
            error.message =
                "bounds bottom latitude must be less than or equal to top "
                "latitude";
            return std::nullopt;
        }

        if (*left > *right) {
            error.message =
                "bounds left longitude must be less than or equal to right "
                "longitude";
            return std::nullopt;
        }
        left = util::max(-180.0, *left);
        right = util::min(180.0, *right);
        result.bounds = LatLngBounds::hull({*bottom, *left}, {*top, *right});
    }

    return result;
}

} // namespace conversion
} // namespace style
} // namespace mbgl
