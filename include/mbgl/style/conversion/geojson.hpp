#pragma once

#include <mbgl/util/geojson.hpp>
#include <mbgl/style/conversion.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace conversion {

// Workaround until https://github.com/mapbox/mapbox-gl-native/issues/5623 is done.
std::optional<GeoJSON> parseGeoJSON(const std::string&, Error&);

template <>
struct Converter<GeoJSON> {
public:
    std::optional<GeoJSON> operator()(const Convertible&, Error&) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
