#include <mbgl/style/conversion/geojson.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion_impl.hpp>

#include <mapbox/geojson.hpp>

namespace mln {
namespace style {
namespace conversion {

std::optional<GeoJSON> Converter<GeoJSON>::operator()(const Convertible& value, Error& error) const {
    return toGeoJSON(value, error);
}

std::optional<GeoJSON> parseGeoJSON(const std::string& value, Error& error) {
    return convertJSON<GeoJSON>(value, error);
}

std::string stringifyGeoJSON(const GeoJSON& geojson) {
    return mapbox::geojson::stringify(geojson);
}

} // namespace conversion
} // namespace style
} // namespace mln
