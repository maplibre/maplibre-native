#include <mbgl/style/conversion/coordinate.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<LatLng> Converter<LatLng>::operator()(const Convertible& value, Error& error) const {
    if (!isArray(value) || arrayLength(value) < 2) {
        error.message =
            "coordinate array must contain numeric longitude and latitude "
            "values";
        return std::nullopt;
    }
    // Style spec uses GeoJSON convention for specifying coordinates
    std::optional<double> latitude = toDouble(arrayMember(value, 1));
    std::optional<double> longitude = toDouble(arrayMember(value, 0));

    if (!latitude || !longitude) {
        error.message =
            "coordinate array must contain numeric longitude and latitude "
            "values";
        return std::nullopt;
    }
    if (*latitude < -90 || *latitude > 90) {
        error.message = "coordinate latitude must be between -90 and 90";
        return std::nullopt;
    }
    return LatLng(*latitude, *longitude);
}

} // namespace conversion
} // namespace style
} // namespace mbgl
